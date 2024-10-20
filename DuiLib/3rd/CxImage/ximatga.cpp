﻿/*
 * File:	ximatga.cpp
 * Purpose:	Platform Independent TGA Image Class Loader and Writer
 * 05/Jan/2001 Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.1 07/Jan/2011
 */

#include "ximatga.h"

#if CXIMAGE_SUPPORT_TGA

#include "ximaiter.h"

// Definitions for image types.
#define TGA_Null 0
#define TGA_Map 1
#define TGA_RGB 2
#define TGA_Mono 3
#define TGA_RLEMap 9
#define TGA_RLERGB 10
#define TGA_RLEMono 11
#define TGA_CompMap 32
#define TGA_CompMap4 33

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
bool CxImageTGA::Decode(CxFile *hFile)
{
    if(hFile == NULL) return false;

    TGAHEADER tgaHead;

    cx_try
    {
        if(hFile->Read(&tgaHead,sizeof(tgaHead),1)==0)
            cx_throw("Not a TGA");

        tga_toh(&tgaHead);

        bool bCompressed;
        switch(tgaHead.ImageType)
        {
        case TGA_Map:
        case TGA_RGB:
        case TGA_Mono:
            bCompressed = false;
            break;
        case TGA_RLEMap:
        case TGA_RLERGB:
        case TGA_RLEMono:
            bCompressed = true;
            break;
        default:
            cx_throw("Unknown TGA image type");
        }

        if(tgaHead.ImageWidth==0 || tgaHead.ImageHeight==0 || tgaHead.PixelDepth==0 || tgaHead.CmapLength>256)
            cx_throw("bad TGA header");

        if(tgaHead.PixelDepth!=8 && tgaHead.PixelDepth!=15 && tgaHead.PixelDepth!=16 && tgaHead.PixelDepth!=24 && tgaHead.PixelDepth!=32)
            cx_throw("bad TGA header");

        if(info.nEscape == -1)
        {
            head.biWidth = tgaHead.ImageWidth ;
            head.biHeight= tgaHead.ImageHeight;
            info.dwType = CXIMAGE_FORMAT_TGA;
            return true;
        }

        if(tgaHead.IdLength>0) hFile->Seek(tgaHead.IdLength,SEEK_CUR);  //skip descriptor

        Create(tgaHead.ImageWidth, tgaHead.ImageHeight, tgaHead.PixelDepth, CXIMAGE_FORMAT_TGA);
#if CXIMAGE_SUPPORT_ALPHA	// <vho>
        if(tgaHead.PixelDepth==32) AlphaCreate();  // Image has alpha channel
#endif //CXIMAGE_SUPPORT_ALPHA

        if(!IsValid()) cx_throw("TGA Create failed");

        if(info.nEscape) cx_throw("Cancelled");  // <vho> - cancel decoding

        if(tgaHead.CmapType != 0)   // read the palette
        {
            rgb_color pal[256];
            hFile->Read(pal,tgaHead.CmapLength*sizeof(rgb_color), 1);
            for(int32_t i=0; i<tgaHead.CmapLength; i++) SetPaletteColor((uint8_t)i,pal[i].b,pal[i].g,pal[i].r);
        }

        if(tgaHead.ImageType == TGA_Mono || tgaHead.ImageType == TGA_RLEMono)
            SetGrayPalette();

        // Bits 4 & 5 of the Image Descriptor byte control the ordering of the pixels.
        bool bXReversed = ((tgaHead.ImagDesc & 16) == 16);
        bool bYReversed = ((tgaHead.ImagDesc & 32) == 32);

        CImageIterator iter(this);
        uint8_t rleLeftover = 255; //for images with illegal packet boundary
        uint8_t* pDest;
        for(int32_t y=0; y < tgaHead.ImageHeight; y++)
        {

            if(info.nEscape) cx_throw("Cancelled");  // <vho> - cancel decoding

            if(hFile == NULL || hFile->Eof()) cx_throw("corrupted TGA");

            if(bYReversed) pDest = iter.GetRow(tgaHead.ImageHeight-y-1);
            else pDest = iter.GetRow(y);

            if(bCompressed) rleLeftover = ExpandCompressedLine(pDest,&tgaHead,hFile,tgaHead.ImageWidth,y,rleLeftover);
            else ExpandUncompressedLine(pDest,&tgaHead,hFile,tgaHead.ImageWidth,y,0);
        }

        if(bXReversed) Mirror();

#if CXIMAGE_SUPPORT_ALPHA
        if(bYReversed && tgaHead.PixelDepth==32) AlphaFlip();  //<lioucr>
#endif //CXIMAGE_SUPPORT_ALPHA

    } cx_catch
    {
        if(strcmp(message,"")) strncpy(info.szLastError,message,255);
        return false;
    }
    return true;
}
////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
bool CxImageTGA::Encode(CxFile * hFile)
{
    if(EncodeSafeCheck(hFile)) return false;

    if(head.biBitCount<8)
    {
        strcpy(info.szLastError,"Bit depth must be 8 or 24");
        return false;
    }

    TGAHEADER tgaHead;

    tgaHead.IdLength = 0;				// Image ID Field Length
    tgaHead.CmapType = GetPalette()!=0; // Color Map Type
    tgaHead.ImageType = (head.biBitCount == 8) ? (uint8_t)TGA_Map : (uint8_t)TGA_RGB; // Image Type

    tgaHead.CmapIndex=0;				// First Entry Index
    tgaHead.CmapLength=(head.biBitCount == 8) ? 256 : 0;   // Color Map Length
    tgaHead.CmapEntrySize=(head.biBitCount == 8) ? (uint8_t)24 : (uint8_t)0; // Color Map Entry Size

    tgaHead.X_Origin=0;					// X-origin of Image
    tgaHead.Y_Origin=0;					// Y-origin of Image
    tgaHead.ImageWidth=(uint16_t)head.biWidth;		// Image Width
    tgaHead.ImageHeight=(uint16_t)head.biHeight;	// Image Height
    tgaHead.PixelDepth=(uint8_t)head.biBitCount;	// Pixel Depth
    tgaHead.ImagDesc=0;					// Image Descriptor

    if(pAlpha && head.biBitCount==24) tgaHead.PixelDepth=32;

    tga_toh(&tgaHead);
    hFile->Write(&tgaHead,sizeof(TGAHEADER),1);
    tga_toh(&tgaHead);

    if(head.biBitCount==8)
    {
        rgb_color pal[256];
        RGBQUAD* ppal = GetPalette();
        for(int32_t i=0; i<256; i++)
        {
            pal[i].r = ppal[i].rgbBlue;
            pal[i].g = ppal[i].rgbGreen;
            pal[i].b = ppal[i].rgbRed;
        }
        hFile->Write(&pal,256*sizeof(rgb_color),1);
    }

    CImageIterator iter(this);
    uint8_t* pDest;
    if(pAlpha==0 || head.biBitCount==8)
    {
        for(int32_t y=0; y < tgaHead.ImageHeight; y++)
        {
            pDest = iter.GetRow(y);
            hFile->Write(pDest,tgaHead.ImageWidth * (head.biBitCount >> 3),1);
        }
    }
    else
    {
        pDest = (uint8_t*)malloc(4*tgaHead.ImageWidth);
        RGBQUAD c;
        for(int32_t y=0; y < tgaHead.ImageHeight; y++)
        {
            for(int32_t x=0, x4=0; x<tgaHead.ImageWidth; x++, x4+=4)
            {
                c = BlindGetPixelColor(x,y);
                pDest[x4+0]=c.rgbBlue;
                pDest[x4+1]=c.rgbGreen;
                pDest[x4+2]=c.rgbRed;
#if CXIMAGE_SUPPORT_ALPHA	// <vho>
                pDest[x4+3]=AlphaGet(x,y);
#else
                pDest[x4+3]=0;
#endif //CXIMAGE_SUPPORT_ALPHA
            }
            hFile->Write(pDest,4*tgaHead.ImageWidth,1);
        }
        free(pDest);
    }
    return true;
}
////////////////////////////////////////////////////////////////////////////////
#endif // CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
uint8_t CxImageTGA::ExpandCompressedLine(uint8_t* pDest,TGAHEADER* ptgaHead,CxFile *hFile,int32_t width, int32_t y, uint8_t rleLeftover)
{
    uint8_t rle;
    int32_t filePos=0;
    for(int32_t x=0; x<width;)
    {
        if(rleLeftover != 255)
        {
            rle = rleLeftover;
            rleLeftover = 255;
        }
        else
        {
            hFile->Read(&rle,1,1);
        }
        if(rle & 128)    // RLE-Encoded packet
        {
            rle -= 127; // Calculate real repeat count.
            if((x+rle)>width)
            {
                rleLeftover = (uint8_t)(128 + (rle - (width - x) - 1));
                filePos = hFile->Tell();
                rle = (uint8_t)(width - x);
            }
            switch(ptgaHead->PixelDepth)
            {
            case 32:
            {
                RGBQUAD color;
                hFile->Read(&color,4,1);
                for(int32_t ix = 0; ix < rle; ix++)
                {
                    memcpy(&pDest[3*ix],&color,3);
#if CXIMAGE_SUPPORT_ALPHA	// <vho>
                    AlphaSet(ix+x,y,color.rgbReserved);
#endif //CXIMAGE_SUPPORT_ALPHA
                }
                break;
            }
            case 24:
            {
                rgb_color triple;
                hFile->Read(&triple,3,1);
                for(int32_t ix = 0; ix < rle; ix++) memcpy(&pDest[3*ix],&triple,3);
                break;
            }
            case 15:
            case 16:
            {
                uint16_t pixel;
                hFile->Read(&pixel,2,1);
                rgb_color triple;
                triple.r = (uint8_t)((pixel & 0x1F) * 8);       // red
                triple.g = (uint8_t)((pixel >> 2) & 0x0F8);     // green
                triple.b = (uint8_t)((pixel >> 7) & 0x0F8);     // blue
                for(int32_t ix = 0; ix < rle; ix++)
                {
                    memcpy(&pDest[3*ix],&triple,3);
                }
                break;
            }
            case 8:
            {
                uint8_t pixel;
                hFile->Read(&pixel,1,1);
                for(int32_t ix = 0; ix < rle; ix++) pDest[ix] = pixel;
            }
            }
            if(rleLeftover!=255) hFile->Seek(filePos, SEEK_SET);
        }
        else     // Raw packet
        {
            rle += 1; // Calculate real repeat count.
            if((x+rle)>width)
            {
                rleLeftover = (uint8_t)(rle - (width - x) - 1);
                rle = (uint8_t)(width - x);
            }
            ExpandUncompressedLine(pDest,ptgaHead,hFile,rle,y,x);
        }
        if(head.biBitCount == 24)	pDest += rle*3;
        else pDest += rle;
        x += rle;
    }
    return rleLeftover;
}
////////////////////////////////////////////////////////////////////////////////
void CxImageTGA::ExpandUncompressedLine(uint8_t* pDest,TGAHEADER* ptgaHead,CxFile *hFile,int32_t width, int32_t y, int32_t xoffset)
{
    switch(ptgaHead->PixelDepth)
    {
    case 8:
        hFile->Read(pDest,width,1);
        break;
    case 15:
    case 16:
    {
        uint8_t* dst=pDest;
        uint16_t pixel;
        for(int32_t x=0; x<width; x++)
        {
            hFile->Read(&pixel,2,1);
            *dst++ = (uint8_t)((pixel & 0x1F) * 8);       // blue
            *dst++ = (uint8_t)((pixel >> 2) & 0x0F8);     // green
            *dst++ = (uint8_t)((pixel >> 7) & 0x0F8);     // red
        }
        break;
    }
    case 24:
        hFile->Read(pDest,3*width,1);
        break;
    case 32:
    {
        uint8_t* dst=pDest;
        for(int32_t x=0; x<width; x++)
        {
            RGBQUAD pixel;
            hFile->Read(&pixel,4,1);
            *dst++ = pixel.rgbBlue;
            *dst++ = pixel.rgbGreen;
            *dst++ = pixel.rgbRed;
#if CXIMAGE_SUPPORT_ALPHA	// <vho>
            AlphaSet(x+xoffset,y,pixel.rgbReserved); //alpha
#endif //CXIMAGE_SUPPORT_ALPHA
        }
        break;
    }
    }
}
////////////////////////////////////////////////////////////////////////////////
void CxImageTGA::tga_toh(TGAHEADER* p)
{
    p->CmapIndex = m_ntohs(p->CmapIndex);
    p->CmapLength = m_ntohs(p->CmapLength);
    p->X_Origin = m_ntohs(p->X_Origin);
    p->Y_Origin = m_ntohs(p->Y_Origin);
    p->ImageWidth = m_ntohs(p->ImageWidth);
    p->ImageHeight = m_ntohs(p->ImageHeight);
}
////////////////////////////////////////////////////////////////////////////////
#endif 	// CXIMAGE_SUPPORT_TGA
