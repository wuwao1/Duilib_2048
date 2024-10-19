#pragma once
#include <map>
namespace DuiLib
{
using CreateClass = std::shared_ptr<CControlUI>(*)();
using MAP_DUI_STRING_CTRATECLASS = std::map<faw::string_t, CreateClass>;

class UILIB_API CControlFactory
{
public:
    std::shared_ptr<CControlUI> CreateControl(faw::string_t strClassName);
    void RegistControl(faw::string_t strClassName, CreateClass pFunc);

public:
    static std::shared_ptr<CControlFactory> GetInstance();

    CControlFactory();   // 构造函数私有，防止外部直接创建实例
    ~CControlFactory();  // 析构函数私有，防止外部直接删除实例
private:

    // 禁止拷贝构造函数和赋值运算符
    CControlFactory(const CControlFactory&) = delete;
    CControlFactory& operator=(const CControlFactory&) = delete;

private:
    MAP_DUI_STRING_CTRATECLASS m_mapControl;
};

#define DECLARE_DUICONTROL(class_name)\
public:\
	static std::shared_ptr<CControlUI> CreateControl();

#define IMPLEMENT_DUICONTROL(class_name)\
	std::shared_ptr<CControlUI> class_name::CreateControl()\
	{ return std::make_shared<class_name>() ; }

#define REGIST_DUICONTROL(class_name)\
	CControlFactory::GetInstance()->RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);

#define INNER_REGISTER_DUICONTROL(class_name)\
	RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);
}