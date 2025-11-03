#include <iostream>
#include "cpp_unicode.h"

/***** MultibyteConvertor *****/

MultibyteConvertor::MultibyteConvertor(unsigned SrcPage, const std::string& SrcStr) :
  m_SrcSize(SrcStr.length() + 1)
{
  m_LastPage = 0;
  Plt_ConvertToUnicode(const_cast<char*>(SrcStr.c_str()), SrcPage);
}

MultibyteConvertor::MultibyteConvertor(const std::string& SrcStr) :
  MultibyteConvertor(Plt_GetSystemCodePage(), SrcStr)
{
}

MultibyteConvertor::MultibyteConvertor(const std::wstring& SrcWStr) :
  m_SrcSize(SrcWStr.length() + 1)
{
  m_LastPage = 0;
  m_StrUnicode.reset(new wchar_t[m_SrcSize]);
  memcpy(m_StrUnicode.get(), SrcWStr.c_str(), sizeof(wchar_t) * m_SrcSize);
}

MultibyteConvertor::~MultibyteConvertor()
{
}

/*Сохранение заданной кодовой страницы в качестве последней рабочей*/
void MultibyteConvertor::SaveLastPageID(unsigned PageID)
{
  m_LastPage = PageID;
}

char* MultibyteConvertor::ToPage(unsigned DstPage)
{
  if (DstPage != m_LastPage) Plt_ConvertFromUnicode(DstPage);
  SaveLastPageID(DstPage);
  return m_StrMultibyte.get();
}

char* MultibyteConvertor::ToSystemPage()
{
  unsigned dstPage = Plt_GetSystemCodePage();
  if (dstPage != m_LastPage) Plt_ConvertFromUnicode(dstPage);
  SaveLastPageID(dstPage);
  return m_StrMultibyte.get();
}

wchar_t* MultibyteConvertor::ToUnicode()
{
  return m_StrUnicode.get();
}

/***** *****/

#ifdef UNICODE
std::wstring ToSystemString(const std::string& Str)
{
  MultibyteConvertor cvt(Str);
  std::wstring result = cvt.ToUnicode();
  return result;
}

std::string FromSystemString(const std::wstring& Str)
{
  MultibyteConvertor cvt(Str);
  std::string result = cvt.ToSystemPage();
  return result;
}
#else
std::string ToSystemString(const std::string& Str)
{
  return Str;
}
std::string FromSystemString(const std::string& Str)
{
  return Str;
}
#endif
