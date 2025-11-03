#pragma once

#include <cstdlib>
#include <cstring>
#include <string>
#include <memory>

#if defined(WINNT) || defined(_WINDOWS) || defined(windows_x86) || defined(windows_x64)
  #if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4996)
  #endif
  //Кодировка
  #ifndef CPP_UNICODE_SYSCODEPAGE
    #define CPP_UNICODE_SYSCODEPAGE 1251
  #endif
#endif
#ifdef linux
  //Кодировка
  #ifndef CPP_UNICODE_SYSCODEPAGE
    #define CPP_UNICODE_SYSCODEPAGE "UTF-8"
  #endif
  #define CPP_UNICODE_UNICODEPAGE "UNICODE"
#endif
#ifdef apple
  //Кодировка
  #ifndef CPP_UNICODE_SYSCODEPAGE
    #define CPP_UNICODE_SYSCODEPAGE "UTF-8"
  #endif
  #define CPP_UNICODE_UNICODEPAGE "UNICODE"
#endif

#define EMPTY_CODEPAGE_ID 0

class MultibyteConvertor
{
public:
  MultibyteConvertor(unsigned SrcPage, const std::string& SrcStr);
  MultibyteConvertor(const std::string& SrcStr);
  MultibyteConvertor(const std::wstring& SrcWStr);
  ~MultibyteConvertor();
private:
  std::size_t m_SrcSize;                         //количество символов в рабочей строке (включая терминальный 0)
  unsigned m_LastPage;                           //идентификатор последней рабочей кодовой страницы
  std::unique_ptr<wchar_t> m_StrUnicode;         //рабочая строка в UNICODE
  std::unique_ptr<char> m_StrMultibyte;          //рабочая строка в последней рабочей кодировке
private:
    //Функции, зависящие от платформы
  /*Получение системной кодировки*/
  unsigned Plt_GetSystemCodePage();
  /*Преобразование рабочей строки из заданной кодировки в UNICODE*/
  void Plt_ConvertToUnicode(char* SrcStr, unsigned FromPageID);
  /*Преобразование рабочей строки из UNICODE в заданную кодировку*/
  void Plt_ConvertFromUnicode(unsigned ToPageID);
private:
  /*Сохранение заданной кодовой страницы в качестве последней рабочей*/
  void SaveLastPageID(unsigned PageID);
public:
  char* ToPage(unsigned DstPage);
  char* ToSystemPage();
  wchar_t* ToUnicode();
};

/***** Работа с вызовами системных функций *****/
// * Для работы с ситемными функциями использовать SYS_STRTYPE, SYS_CHARTYPE
// * Для сохранения результата работы системной функции в std::string использовать FromSystemString
// * Для получения SYS_STRTYPE из std::string использовать ToSystemString

#ifdef UNICODE
#define SYS_STRTYPE std::wstring
#define SYS_CHARTYPE wchar_t
std::wstring ToSystemString(const std::string& Str);
std::string FromSystemString(const std::wstring& Str);
#else
#define SYS_STRTYPE std::string
#define SYS_CHARTYPE char
std::string ToSystemString(const std::string& Str);
std::string FromSystemString(const std::string& Str);
#endif

#if defined(WINNT)
  #if defined(_MSC_VER)
    #pragma warning(pop)
  #endif
#endif
