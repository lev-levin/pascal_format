#ifndef CPP_FORMAT_H
#define CPP_FORMAT_H

#include <cctype>
#include <string>
#include <thread>
#include <exception>
#include <vector>
#include <memory>
#include <iostream>

#include "dwmutex.h"
#include "cpp_unicode.h"

/***** Вспомогательные функции трассировки работы утилит форматирования и отладчика *****/

/*Вывод сведений в стандартный файл "dbg_trace.nfo"*/
void _Dbg_Trace(const std::string& Msg);

/***** ECppFmtTypeException *****/

#if defined(_WIN32) || defined(_WIN64)
  #if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4996)
  #endif
  #ifndef _GLIBCXX_TXN_SAFE_DYN
    #define _GLIBCXX_TXN_SAFE_DYN
  #endif
  #ifndef _GLIBCXX_USE_NOEXCEPT
    #define _GLIBCXX_USE_NOEXCEPT
  #endif
  #define EXCEPTION_WHAT_SUFFIX _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT
#endif
#ifdef linux
  #define EXCEPTION_WHAT_SUFFIX  noexcept override
#endif
#ifdef apple
#define EXCEPTION_WHAT_SUFFIX  noexcept override
#endif

class ECppFmtTypeException : public std::exception {
public:
  ECppFmtTypeException(const std::string& FmtSpec, const std::string& TypeName) {
    FMessage = "Format specifier '%" + FmtSpec + "' not supported for type '" + TypeName + "'";
  }
private:
  std::string FMessage;
public:
  const char* what() const EXCEPTION_WHAT_SUFFIX {
    return FMessage.c_str();
  }
};

/***** FmtItem *****/

/*Оболочка для добавления в форматированный вывод своих типов*/
//чтобы добавить свой тип, нужно создать соответствующую специализацию шаблона template<> class FmtItem<AnyClass>
//и реализовать в ней публичные методы:
//  std::string Type()
//  void SetValue(const AnyClass& Value) и
//  std::string ShowValue(const std::string& FmtSpec, int Len, int Prec)
//  AnyClass GetValue() const
//после этого можно будет выполнять код вида:
//  AnyClass aClass;
//  FormatStr("Class value is %s", FmtArgs<<aClass);
//  (можно использовать любые символы формата, поддерживаемые методом FmtItem<AnyClass>::ShowValue)
template <typename T>
class FmtItem {
public:
  /*Тип отображаемого значения*/
  std::string Type();
  /*Установка отображаемого значения*/
  void SetValue(const T& Value);
  /*Чтение отображаемого значения*/
  T GetValue() const;
  /*Преобразование отображаемого значения в строку*/
  std::string ShowValue(const std::string& FmtSpec, int Len, int Prec);
};

/***** CFmtItem_Abstract *****/

/*Базовый абстрактный класс для визуализации величин*/
class CFmtItem_Abstract
{
public:
  virtual ~CFmtItem_Abstract() {}
public:
  /*Обозначение типа визуализируемой величины*/
  virtual std::string type() = 0;
  /*Функция преобразующая значение, переданное объекту в конструкторе, в строку*/
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) = 0;
  /*Создание копии объекта*/
  virtual CFmtItem_Abstract* clone() const = 0;
};

/*Заготовка для отслеживания значений переменных прочих типов*/
template<typename T>
class CFmtItem_User : public CFmtItem_Abstract
{
public:
  CFmtItem_User(const T& Value) {
    viewer.SetValue(Value);
  }
private:
  FmtItem<T> viewer;
public:
  virtual std::string type() override { return viewer.Type(); }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    return viewer.ShowValue(FmtSpec, Len, Prec);
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_User<T>(viewer.GetValue());
  }
};

/***** CFmtVariant *****/

/*Класс, выполняющий преобразование значения произвольного типа в строку*/
//каждый конструктор класса принимает соответствующую ему преобразуемую величину
class CFmtVariant {
  friend class CFmtVector;
public:
  ~CFmtVariant() {
    delete m_Viewer;
  }
private:
  CFmtVariant() : m_Viewer(nullptr) {}
public:
  CFmtVariant(bool Value);
  CFmtVariant(char Value);
  CFmtVariant(wchar_t Value);
  CFmtVariant(unsigned char Value);
  CFmtVariant(int Value);
  CFmtVariant(short Value);
  #ifdef _NATIVE_WCHAR_T_DEFINED //предотвращение ошибок компиляции, связанных с опцией компилятора msvc -Zc:wchar_t-
  CFmtVariant(unsigned short Value);
  #endif // _NATIVE_WCHAR_T_DEFINED
  CFmtVariant(long Value);
  CFmtVariant(long long Value);
  CFmtVariant(unsigned Value);
  CFmtVariant(unsigned long Value);
  CFmtVariant(unsigned long long Value);
  CFmtVariant(float Value);
  CFmtVariant(double Value);
  CFmtVariant(long double Value);
  CFmtVariant(char* Value);
  CFmtVariant(const char* Value);
  CFmtVariant(const std::string& Value);
  CFmtVariant(wchar_t* Value);
  CFmtVariant(const wchar_t* Value);
  CFmtVariant(const std::wstring& Value);
  CFmtVariant(void* Value);
  CFmtVariant(const void* Value);
  CFmtVariant(const std::thread::id& Value);
  CFmtVariant(const std::thread& Value);
  CFmtVariant(const std::exception& Value);
  template <typename T>
  CFmtVariant(const T& Value) : m_Viewer(new CFmtItem_User<T>(Value)) {}
private:
  CFmtItem_Abstract* m_Viewer;
public:
  /*Обозначение типа преобразуемой величины*/
  std::string type() const { return m_Viewer->type(); }
  /*Получение строки для преобразуемой величины*/
  std::string str(const std::string& FmtSpec, int Len, int Prec) const;
  /*Создание копии объекта*/
  CFmtVariant* clone() const;
};

/***** CFmtVector *****/

class CFmtVector : public std::vector<CFmtVariant*> {
public:
  CFmtVector() : std::vector<CFmtVariant*>() {}
  CFmtVector(const CFmtVector& FmtSource) : std::vector<CFmtVariant*>() {
    copyItems(FmtSource);
  }
  ~CFmtVector()
  {
    clearItems();
  }
private:
  void clearItems() {
    for (CFmtVariant* val : *this) delete val;
    clear();
  }
  void copyItems(const CFmtVector& FmtSource) {
    for (CFmtVariant* fmtVar : FmtSource) {
      push_back(fmtVar->clone());
    }
  }
public:
  template<typename T>
  CFmtVector& operator<<(const T& Value)
  {
    CFmtVariant* newVal = new CFmtVariant(Value);
    push_back(newVal);
    return *this;
  }
  CFmtVector operator=(const CFmtVector& FmtSource) {
    clearItems();
    copyItems(FmtSource);
    return *this;
  }
};

#define FmtArgs_Type const CFmtVector&
#define FmtArgs CFmtVector()

/***** *****/

/*Элемент разбора входной строки форматированного вывода*/
struct TFmtPair {
  TFmtPair(const std::string& StrFirst, const std::string& StrSecond) : first(StrFirst), second(StrSecond) {}
  std::string first;
  std::string second;
};
/*Полный разбор входной строки форматированного вывода*/
typedef std::vector<TFmtPair> TFmtCompiledString;
/*Полная информация о входной строке форматированного вывода*/
struct TFmtStrData {
  TFmtStrData(const std::string& FmtStr, const TFmtCompiledString& FmtData) : source(FmtStr), format(FmtData) {}
  std::string source;
  TFmtCompiledString format;
  bool operator ==(const std::string& FmtStr) const { return source == FmtStr; }
};
/*Коллекция сохраненных данных о строках форматированного вывода*/
class TFmtDictionary
{
private:
  std::vector<TFmtStrData> m_Dictionary;         //набор сохраненных строк форматированного вывода
  CMutexItem m_DictAccess;                       //управление доступом к набору сохраненных строк форматированного вывода
public:
  /*Поиск сведений о заданной строке форматированного вывода*/
  bool StrFormatExists(const std::string& Str, TFmtCompiledString& Format);
  /*Сохранение в словаре сведений о строке форматированного вывода*/
  void AddStrFormat(const std::string& Str, const TFmtCompiledString& Format);
};

/*Разбиение входной строки на пары [<префикс>;<описатель формата>]*/
void buildFmtList(const std::string& FmtString, TFmtCompiledString& Result, TFmtDictionary* FmtDictionary);
/*Получение символьного описателя выводимой величины из строки-полной спецификации формата вывода величины*/
std::string getFormatLetter(const std::string& FmtSpec);
/*Получение строки в результате подстановки значений аргументов в исходную строку*/
std::string FormatStr(const std::string& FmtString, FmtArgs_Type ArgList, TFmtDictionary* FmtDictionary = nullptr);
//SYS_STRTYPE FormatSys(const std::string& FmtString, FmtArgs_Type ArgList, TFmtDictionary* FmtDictionary = nullptr);
std::unique_ptr<char[]> FormatChar(const std::string& FmtString, FmtArgs_Type ArgList, TFmtDictionary* FmtDictionary = nullptr);

#if defined(_WIN32) || defined(_WIN64)
  #if defined(_MSC_VER)
    #pragma warning(pop)
  #endif
#endif

/*Вывод сведений в стандартный файл "dbg_trace.nfo"*/
void _Dbg_Trace(const std::string& Msg, FmtArgs_Type MsgItems);

#endif // CPP_FORMAT_H
