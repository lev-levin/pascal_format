/*ПЕРЕКОДИРОВКА СТРОК: РЕАЛИЗАЦИЯ ДЛЯ WINDOWS*/

#include "cpp_unicode.h"
#include <windows.h>

/*Получение системной кодировки*/
unsigned MultibyteConvertor::Plt_GetSystemCodePage()
{
  return GetACP();
}

/*Преобразование рабочей строки из заданной кодировки в UNICODE*/
void MultibyteConvertor::Plt_ConvertToUnicode(char* SrcStr, unsigned FromPageID)
{
  m_StrUnicode.reset(new wchar_t[m_SrcSize]);
  MultiByteToWideChar(FromPageID, 0, SrcStr, -1, m_StrUnicode.get(), m_SrcSize);
}

/*Преобразование рабочей строки из UNICODE в заданную кодировку*/
void MultibyteConvertor::Plt_ConvertFromUnicode(unsigned ToPageID)
{
  unsigned dstPageId = (ToPageID == 0) ? Plt_GetSystemCodePage()  : ToPageID;
  size_t dstSize = m_SrcSize * sizeof(wchar_t);
  m_StrMultibyte.reset(new char[dstSize]);
  WideCharToMultiByte(dstPageId, 0, m_StrUnicode.get(), -1, m_StrMultibyte.get(), dstSize, NULL, NULL);
}
