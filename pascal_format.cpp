#include "stdafx.h"

#include "pascal_format.h"
#include "pasfmt_thread_support.h"
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <regex>
#include <iostream>
#include <cstring>
#include <cctype>
#include <clocale>
#include <algorithm>

/**************************************************************************************************/
//%c - символ
//%d - целое число (int;long;long long) в десятичном формате (символы слева - нули)
//     беззнаковое целое (uint;ulong;ulong long) в десятичном формате (символы слева - нули)
//%D - целое число (int;long;long long) в десятичном формате (символы слева - пробелы)
//     беззнаковое целое (uint;ulong;ulong long) в десятичном формате (символы слева - пробелы)
//%ld - длинное целое (long) в десятичном формате (символы слева - нули)
//%lD - длинное целое (long) в десятичном формате (символы слева - пробелы)
//%Ld - 64-битное целое (long long) в десятичном формате (символы слева - нули)
//%LD - 64-битное целое (ulong long) в десятичном формате (символы слева - пробелы)
//%u - беззнаковое целое (uint) в десятичном формате (символы слева - нули)
//%U - беззнаковое целое (uint) в десятичном формате (символы слева - пробелы)
//%lu - беззнаковое длинное целое (ulong) в десятичном формате (символы слева - нули)
//%lU - беззнаковое длинное целое (ulong) в десятичном формате (символы слева - пробелы)
//%Lu - беззнаковое 64-битное целое (ulong long) в десятичном формате (символы слева - нули)
//%LU - беззнаковое 64-битное целое (ulong long) в десятичном формате (символы слева - пробелы)
//%x - символ в 16-ричном формате
//     беззнаковое целое в 16-ричном формате
//     строка C в 16-ричном формате (параметр Len должен указывать длину строки или 0 - до символа '\0';
//       параметр Prec должен указывать количество отображаемых байт строки или 0 - Len байт)
//     строка C++ в 16-ричном формате (параметр Len должен указывать длину строки или 0 - вся строка;
//       параметр Prec должен указывать количество отображаемых байт строки или 0 - Len байт)
//%lx - беззнаковое длинное целое (ulong) в 16-ричном формате
//%Lx - беззнаковое 64-битное целое (ulong long) в 16-ричном формате
//%b - беззнаковое целое в виде битовой строки
//%lb - беззнаковое длинное целое (ulong) в виде битовой строки
//%Lb - беззнаковое 64-битное целое (ulong long) в виде битовой строки
//%f - число с плавающей точкой (float;double) в десятичном формате
//%lf - число с плавающей точкой (double) в десятичном формате
//%Lf - число с плавающей точкой (long double) в десятичном формате
//%s - указатель на строку в виде строки
//     строка C++ (std::string) в виде строки
//     логическое значение в виде строки (для целочисленных типов)
//     логическое значение "не NULL" (для указателя)
//     описание исключения std::exception в виде строки
//%is - логическое значение в виде строки (для типа int: не ноль/ноль)
//%ls - логическое значение в виде строки (для типа long: не ноль/ноль)
//%Ls - логическое значение в виде строки (для типа long long: не ноль/ноль)
//%rs - логическое значение в виде строки (для типа void*: не NULL/NULL)
//%p - указатель
//%t - идентификатор потока
//%T - время в формате чч-мм-сс (для типа long long aka time_t)
/**************************************************************************************************/

template<typename T>
std::string BoolString(void* Data)
{
  T intValue = *(static_cast<T*>(Data));
  return (intValue) ? "<T>" : "<F>";
}

std::string HexStringForward(void* Data, std::size_t BytesCount)
{
  const char hexDigits[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  unsigned char* letter = static_cast<unsigned char*>(Data);
  std::string result = "";
  for(std::size_t i = 0; i < BytesCount; i++, letter++) {
    result = result + hexDigits[((*letter) & 0xF0) >> 4] + hexDigits[(*letter) & 0xF];
  }
  return result;
}

std::string HexStringReverse(void* Data, std::size_t BytesCount, const std::string& BytesDelimiter)
{
  const char hexDigits[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  unsigned char* letter = static_cast<unsigned char*>(Data);
  letter += (BytesCount - 1);
  std::string result = BytesDelimiter;
  for(std::size_t i = 0; i < BytesCount; i++, letter--) {
    result = result + hexDigits[((*letter) & 0xF0) >> 4] + hexDigits[(*letter) & 0xF] + BytesDelimiter;
  }
  return result;
}

std::string BitString(unsigned long long Data, std::size_t BitsCount)
{
  std::string result = "";
  for (std::size_t i = 0; i < sizeof(unsigned long long); i++) {
    result = std::string(1, '0' + (Data & 1)) + result;
    Data = Data >> 1;
  }
  if (result.length() < BitsCount) result = std::string(BitsCount - result.length(), '0') + result;
  return result.substr(result.length() - BitsCount, BitsCount);
}

template<typename FloatType>
std::string FnStrCvt(FloatType A)
{
  const FloatType C_EPSILON = 0.1;
  std::string result = "";
  int digCount;
  FloatType mult;
  for (digCount = 0, mult = 1; mult < (A - C_EPSILON); digCount += 1, mult *= 10);
  if (A > (mult - C_EPSILON)) digCount += 1;
    else mult /= 10;
  for (int i = 0; i < digCount; i++) {
    int digit = static_cast<int>(A / mult);
    result = result + std::to_string(digit);
    A -= (digit * mult);
    mult /= 10;
  }
  return (result == "") ? "0" : result;
}

#ifdef _WIN32
  #pragma warning(push)
  #pragma warning(disable : 4018)
#endif
template<typename FloatType>
std::string FloatTypeToString(void* FloatPtr, int Len, int Prec)
{
  FloatType floatValue = *static_cast<FloatType*>(FloatPtr);

  double intPart;
  double fracPart = modf(floatValue, &intPart);
  intPart = fabs(intPart);
  fracPart = fabs(fracPart);
  if (Prec == 0) Prec = 6;
  for (int i = 0; i < Prec; i ++) fracPart *= 10;
  fracPart = round(fracPart);

  std::string intResult = FnStrCvt(intPart);
  std::string fracResult = FnStrCvt(fracPart);
  if (floatValue < 0) intResult = "-" + intResult;

  std::size_t fracLen = (Prec > 0) ? Prec : 1;
  std::size_t intLen = (Len > (fracLen+1)) ? Len - fracLen - 1 : 1;
  if (intLen < intResult.size()) intLen = intResult.size();

  intResult = std::string(intLen - intResult.size(), ' ') + intResult;
  fracResult = std::string(fracResult, 0, fracLen);
  if (fracLen > fracResult.size()) std::string(fracLen - fracResult.size(), '0');

  return intResult + "." + fracResult;
}
#ifdef _WIN32
  #pragma warning(pop)
#endif

/**************************************************************************************************/

std::string FnStringToStr(char** SrcStrPtr, std::size_t ResLen)
{
  char* srcStr = *SrcStrPtr;
  if (ResLen == 0) ResLen = strlen(srcStr);
  std::unique_ptr<char[]> resStr(new char[ResLen+1]);
  std::size_t srcLen = strlen(srcStr);
  resStr.get()[ResLen] = '\0';
  if (srcLen > ResLen) memcpy(resStr.get(), srcStr, ResLen);
  else if (srcLen < ResLen) {
    std::unique_ptr<char[]> tailStr(new char[ResLen-srcLen+1]);
    tailStr.get()[ResLen-srcLen] = '\0';
    memset(tailStr.get(), ' ', ResLen-srcLen);
    strcpy(resStr.get(), srcStr);
    strcat(resStr.get(), tailStr.get());
  }
  else strcpy(resStr.get(), srcStr);
  return std::string(resStr.get());
}

std::string FnStringToHex(char** SrcStrPtr, std::size_t SrcStrSize, std::size_t ResLen)
{
  if (SrcStrSize == 0) SrcStrSize = strlen(*SrcStrPtr);
  if (ResLen == 0) ResLen = SrcStrSize;
  size_t bytesCount = (ResLen < SrcStrSize) ? ResLen : SrcStrSize;
  std::string resStr = HexStringForward(*SrcStrPtr, bytesCount);
  std::string result = "|";
  for (size_t i = 0; i < resStr.length(); i += 2) {
    result = result + resStr.substr(i, 2) + "|";
  }
  if (bytesCount < SrcStrSize) result = result + "...";
  return result;
}

/***** CFmtItem_Int *****/

class CFmtItem_Int : public CFmtItem_Abstract
{
public:
  CFmtItem_Int(int Value) : m_CvtValue(Value) {}
private:
  int m_CvtValue;
public:
  virtual std::string type() override { return "int"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Prec;
    std::string result = "";
    if (FmtSpec == "d") {
      std::string val = std::to_string(m_CvtValue);
      int headLen = Len - (int)val.size();
      result = (headLen <= 0) ? val : std::string(headLen, '0') + val;
    }
    else if (FmtSpec == "D") {
      std::string val = std::to_string(m_CvtValue);
      int headLen = Len - (int)val.size();
      result = (headLen <= 0) ? val : std::string(headLen, ' ') + val;
    }
    else if ((FmtSpec == "s") || (FmtSpec == "is")) result = BoolString<int>(&m_CvtValue);
    else throw ECppFmtTypeException(FmtSpec, "int");
    return result;
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_Int(m_CvtValue);
  }
};

/***** CFmtItem_Unsigned *****/

class CFmtItem_Unsigned : public CFmtItem_Abstract
{
public:
  CFmtItem_Unsigned(unsigned Value) : m_CvtValue(Value) {}
private:
  unsigned m_CvtValue;
public:
  virtual std::string type() override { return "unsigned"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Prec;
    std::string val;
    int headLen;
    if ((FmtSpec == "d") || (FmtSpec == "u")) {
      val = std::to_string(m_CvtValue);
      headLen = Len - (int)val.size();
      return (headLen <= 0) ? val : std::string(headLen, '0') + val;
    }
    else if ((FmtSpec == "D") || (FmtSpec == "U")) {
      val = std::to_string(m_CvtValue);
      headLen = Len - (int)val.size();
      return (headLen <= 0) ? val : std::string(headLen, ' ') + val;
    }
    else if (FmtSpec == "x") return HexStringReverse(&m_CvtValue, sizeof(unsigned), "|");
    else if (FmtSpec == "s") return BoolString<int>(&m_CvtValue);
    else if (FmtSpec == "b") {
      unsigned val = *static_cast<unsigned*>(&m_CvtValue);
      return BitString(val, sizeof(unsigned) * 8);
    }
    else throw ECppFmtTypeException(FmtSpec, "unsigned");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_Unsigned(m_CvtValue);
  }
};

/***** CFmtItem_Bool *****/

class CFmtItem_Bool : public CFmtItem_Abstract
{
public:
  CFmtItem_Bool(char Value) : m_CvtValue(Value) {}
private:
  bool m_CvtValue;
public:
  virtual std::string type() override { return "bool"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if (FmtSpec == "s") return CFmtItem_Int((int)m_CvtValue).str(FmtSpec, Len, Prec);
      else throw ECppFmtTypeException(FmtSpec, "bool");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_Bool(m_CvtValue);
  }
};

/***** CFmtItem_Char *****/

class CFmtItem_Char : public CFmtItem_Abstract
{
public:
  CFmtItem_Char(char Value) : m_CvtValue(Value) {}
private:
  char m_CvtValue;
public:
  virtual std::string type() override { return "char"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if (FmtSpec == "c") return std::string(1, m_CvtValue);
    else if ((FmtSpec == "d") || (FmtSpec == "s")) return CFmtItem_Int((int)m_CvtValue).str(FmtSpec, Len, Prec);
    else throw ECppFmtTypeException(FmtSpec, "char");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_Char(m_CvtValue);
  }
};

/***** CFmtItem_WChar *****/

class CFmtItem_WChar : public CFmtItem_Abstract
{
public:
  CFmtItem_WChar(wchar_t Value) : m_CvtValue(Value) {}
private:
  wchar_t m_CvtValue;
public:
  virtual std::string type() override { return "wchar_t"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Len;
    (void)Prec;
    std::string result;
    if ((FmtSpec == "c") || (FmtSpec == "wc")) {
      std::wstring wstr = std::wstring(1, m_CvtValue);
      MultibyteConvertor wcvt(wstr);
      result = wcvt.ToSystemPage();
    }
    #ifndef _NATIVE_WCHAR_T_DEFINED //с опцией компилятора msvc -Zc:wchar_t- тип wchar_t - это синоним беззнакового целого
    else {                          //поэтому допускаются целочисленные символы форматирования
      std::string digitSpec = FormatStr("%%%d.%dd", FmtArgs<<Len<<Prec);
      uint64_t digitVal = m_CvtValue;
      result = FormatStr(digitSpec, FmtArgs<<digitVal);
    }
    #else                           //без опции компилятора msvc -Zc:wchar_t- тип wchar_t - встроенный
    else throw ECppFmtTypeException(FmtSpec, "wchar_t");
    #endif
    return result;
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_WChar(m_CvtValue);
  }
};

/***** CFmtItem_UChar *****/

class CFmtItem_UChar : public CFmtItem_Abstract
{
public:
  CFmtItem_UChar(unsigned char Value) : m_CvtValue(Value) {}
private:
  unsigned char m_CvtValue;
public:
  virtual std::string type() override { return "unsigned char"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if (FmtSpec == "c") return std::string(1, m_CvtValue);
    else if ((FmtSpec == "d") || (FmtSpec == "s") || (FmtSpec == "x") || (FmtSpec == "b")) {
      return CFmtItem_Unsigned((unsigned)m_CvtValue).str(FmtSpec, Len, Prec);
    }
    else throw ECppFmtTypeException(FmtSpec, "unsigned char");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_UChar(m_CvtValue);
  }
};

/***** CFmtItem_Long *****/

class CFmtItem_Long : public CFmtItem_Abstract
{
public:
  CFmtItem_Long(long Value) : m_CvtValue(Value) {}
private:
  long m_CvtValue;
public:
  virtual std::string type() override { return "long"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Prec;
    std::string result;
    if ((FmtSpec == "d") || (FmtSpec == "ld")) {
      std::string val = std::to_string(m_CvtValue);
      int headLen = Len - (int)val.size();
      result = (headLen <= 0) ? val : std::string(headLen, '0') + val;
    }
    else if ((FmtSpec == "D") || (FmtSpec == "lD")) {
      std::string val = std::to_string(m_CvtValue);
      int headLen = Len - (int)val.size();
      result = (headLen <= 0) ? val : std::string(headLen, ' ') + val;
    }
    else if ((FmtSpec == "s") || (FmtSpec == "ls")) result = BoolString<long int>(&m_CvtValue);
    else throw ECppFmtTypeException(FmtSpec, "long int");
    return result;
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_Long(m_CvtValue);
  }
};

/***** CFmtItem_LongLong *****/

class CFmtItem_LongLong : public CFmtItem_Abstract
{
public:
  CFmtItem_LongLong(long long Value) : m_CvtValue(Value) {}
private:
  long long m_CvtValue;
public:
  virtual std::string type() override { return "long long"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Prec;
    std::string result;
    if ((FmtSpec == "d") || (FmtSpec == "Ld")) {
      std::string val = std::to_string(m_CvtValue);
      int headLen = Len - (int)val.size();
      result = (headLen <= 0) ? val : std::string(headLen, '0') + val;
    }
    else if ((FmtSpec == "D") || (FmtSpec == "LD")) {
      std::string val = std::to_string(m_CvtValue);
      int headLen = Len - (int)val.size();
      result = (headLen <= 0) ? val : std::string(headLen, ' ') + val;
    }
    else if ((FmtSpec == "s") || (FmtSpec == "Ls")) result = BoolString<long long>(&m_CvtValue);
    else throw ECppFmtTypeException(FmtSpec, "long long");
    return result;
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_LongLong(m_CvtValue);
  }
};

/***** CFmtItem_ULong *****/

class CFmtItem_ULong : public CFmtItem_Abstract
{
public:
  CFmtItem_ULong(unsigned long Value) : m_CvtValue(Value) {}
private:
  unsigned long m_CvtValue;
public:
  virtual std::string type() override { return "unsigned long"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Prec;
    std::string val;
    int headLen;
    if ((FmtSpec == "d") || (FmtSpec == "lu")) {
      val = std::to_string(m_CvtValue);
      headLen = Len - (int)val.size();
      return (headLen <= 0) ? val : std::string(headLen, '0') + val;
    }
    else if ((FmtSpec == "D") || (FmtSpec == "lU")) {
      val = std::to_string(m_CvtValue);
      headLen = Len - (int)val.size();
      return (headLen <= 0) ? val : std::string(headLen, ' ') + val;
    }
    else if ((FmtSpec == "x") || (FmtSpec == "lx")) return HexStringReverse(&m_CvtValue, sizeof(unsigned long), "|");
    else if (FmtSpec == "s") return BoolString<unsigned long>(&m_CvtValue);
    else if ((FmtSpec == "b") || (FmtSpec == "lb")) return BitString(m_CvtValue, sizeof(unsigned long) * 8);
    else throw ECppFmtTypeException(FmtSpec, "unsigned long");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_ULong(m_CvtValue);
  }
};

/***** CFmtItem_ULongLong *****/

class CFmtItem_ULongLong : public CFmtItem_Abstract
{
public:
  CFmtItem_ULongLong(unsigned long long Value) : m_CvtValue(Value) {}
private:
  unsigned long long m_CvtValue;
public:
  virtual std::string type() override { return "unsigned long long"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Prec;
    std::string val;
    int headLen;
    if ((FmtSpec == "d") || (FmtSpec == "Lu")) {
      val = std::to_string(m_CvtValue);
      headLen = Len - (int)val.size();
      return (headLen <= 0) ? val : std::string(headLen, '0') + val;
    }
    else if ((FmtSpec == "D") || (FmtSpec == "LU")) {
      val = std::to_string(m_CvtValue);
      headLen = Len - (int)val.size();
      return (headLen <= 0) ? val : std::string(headLen, ' ') + val;
    }
    else if ((FmtSpec == "x") || (FmtSpec == "Lx")) return HexStringReverse(&m_CvtValue, sizeof(unsigned long long), "|");
    else if (FmtSpec == "s") return BoolString<unsigned long long>(&m_CvtValue);
    else if ((FmtSpec == "b") || (FmtSpec == "Lb")) return BitString(m_CvtValue, sizeof(unsigned long long) * 8);
    else throw ECppFmtTypeException(FmtSpec, "unsigned long long");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_ULongLong(m_CvtValue);
  }
};

/***** CFmtItem_Float *****/

class CFmtItem_Float : public CFmtItem_Abstract
{
public:
  CFmtItem_Float(float Value) : m_CvtValue(Value) {}
private:
  float m_CvtValue;
public:
  virtual std::string type() override { return "float"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if (FmtSpec == "f") return FloatTypeToString<float>(&m_CvtValue, Len, Prec);
      else throw ECppFmtTypeException(FmtSpec, "float");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_Float(m_CvtValue);
  }
};

/***** CFmtItem_Double *****/

class CFmtItem_Double : public CFmtItem_Abstract
{
public:
  CFmtItem_Double(double Value) : m_CvtValue(Value) {}
private:
  double m_CvtValue;
public:
  virtual std::string type() override { return "double"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if ((FmtSpec == "f") || (FmtSpec == "lf")) return FloatTypeToString<double>(&m_CvtValue, Len, Prec);
      else throw ECppFmtTypeException(FmtSpec, "double");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_Double(m_CvtValue);
  }
};

/***** CFmtItem_LongDouble *****/

class CFmtItem_LongDouble : public CFmtItem_Abstract
{
public:
  CFmtItem_LongDouble(long double Value) : m_CvtValue(Value) {}
private:
  long double m_CvtValue;
public:
  virtual std::string type() override { return "long double"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if ((FmtSpec == "f") || (FmtSpec == "Lf")) return FloatTypeToString<long double>(&m_CvtValue, Len, Prec);
      else throw ECppFmtTypeException(FmtSpec, "long double");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_LongDouble(m_CvtValue);
  }
};

/***** CFmtItem_PVoid *****/

class CFmtItem_PVoid : public CFmtItem_Abstract
{
public:
  CFmtItem_PVoid(void* Value) : m_CvtValue(Value) {}
private:
  void* m_CvtValue;
public:
  virtual std::string type() override { return "void*"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Len;
    (void)Prec;
    if (FmtSpec == "p") return "[" + HexStringReverse(&m_CvtValue, sizeof(void*), "") + "]";
    if ((FmtSpec == "s") || (FmtSpec == "rs")) {
      int notNull = (m_CvtValue != NULL) ? 1 : 0;
      return BoolString<int>((void*)&notNull);
    }
    else throw ECppFmtTypeException(FmtSpec, "pointer type");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_PVoid(m_CvtValue);
  }
};

/***** CFmtItem_PChar *****/

class CFmtItem_PChar : public CFmtItem_Abstract
{
public:
  CFmtItem_PChar(char* Value) : m_CvtValue(Value) {}
private:
  char* m_CvtValue;
public:
  virtual std::string type() override { return "char*"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if (FmtSpec == "s") return FnStringToStr(&m_CvtValue, Len);
    else if (FmtSpec == "x") return FnStringToHex(&m_CvtValue, Len, Prec);
    else if (FmtSpec == "p") return CFmtItem_PVoid(m_CvtValue).str(FmtSpec, Len, Prec);
    throw ECppFmtTypeException(FmtSpec, "char*");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_PChar(m_CvtValue);
  }
};

/***** CFmtItem_StdString *****/

class CFmtItem_StdString : public CFmtItem_Abstract
{
public:
  CFmtItem_StdString(const std::string& Value) : m_CvtValue(Value) {}
private:
  std::string m_CvtValue;
public:
  virtual std::string type() override { return "std::string"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Len;
    int stdStrLen = (int)m_CvtValue.size();
    if (FmtSpec == "s") return CFmtItem_PChar(const_cast<char*>(m_CvtValue.c_str())).str(FmtSpec, stdStrLen, Prec);
    else if (FmtSpec == "x") {
      std::unique_ptr<char[]> srcPtr(new char[stdStrLen+1]);
      char* srcStr = srcPtr.get();
      srcStr[stdStrLen] = '\0';
      m_CvtValue.copy(srcStr, stdStrLen);
      return CFmtItem_PChar(srcStr).str(FmtSpec, stdStrLen, Prec);
    }
    else throw ECppFmtTypeException(FmtSpec, "std::string");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_StdString(m_CvtValue);
  }
};

/***** CFmtItem_PWChar *****/

class CFmtItem_PWChar : public CFmtItem_Abstract
{
public:
  CFmtItem_PWChar(wchar_t* Value) : m_CvtValue(Value) {}
private:
  wchar_t* m_CvtValue;
public:
  virtual std::string type() override { return "wchar_t*"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if ((FmtSpec == "s") || (FmtSpec == "ws")) {
      std::size_t len = wcslen(m_CvtValue);
      std::unique_ptr<char[]> stdStr(new char[len+1]);
      wcstombs(stdStr.get(), m_CvtValue, len+1);
      char* stdPtr = stdStr.get();
      stdPtr[len] = '\0';
      return CFmtItem_PChar(stdPtr).str(FmtSpec, Len, Prec);
    }
    throw ECppFmtTypeException(FmtSpec, "wchar_t*");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_PWChar(m_CvtValue);
  }
};

/***** CFmtItem_StdWString *****/

class CFmtItem_StdWString : public CFmtItem_Abstract
{
public:
  CFmtItem_StdWString(const std::wstring& Value) : m_CvtValue(Value) {}
private:
  std::wstring m_CvtValue;
public:
  virtual std::string type() override { return "std::wstring"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Len;
    int stdStrLen = (int)m_CvtValue.size();
    if (FmtSpec == "s") return CFmtItem_PWChar(const_cast<wchar_t*>(m_CvtValue.c_str())).str(FmtSpec, stdStrLen, Prec);
      else throw ECppFmtTypeException(FmtSpec, "std::wstring");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_StdWString(m_CvtValue);
  }
};

/***** CFmtItem_StdThreadId *****/

class CFmtItem_StdThreadId : public CFmtItem_Abstract
{
public:
  CFmtItem_StdThreadId(const std::thread::id& Value) : m_CvtValue(Value) {}
private:
  static CPasFmt_ThreadList m_ThreadList;
  const std::thread::id& m_CvtValue;
public:
  virtual std::string type() override { return "std::thread::id"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Len;
    (void)Prec;
    if (FmtSpec == "t") {
      int outLen = (Len < 5) ? 5 : Len;
      std::string fmt = FormatStr("%%%dd", FmtArgs<<outLen);
      return FormatStr(fmt, FmtArgs<<m_ThreadList.ThreadIdToInt(m_CvtValue));
    }
    if (FmtSpec == "x") return HexStringReverse((void*)&m_CvtValue, sizeof(std::thread::id), "");
    else throw ECppFmtTypeException(FmtSpec, "std::thread::id");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_StdThreadId(m_CvtValue);
  }
};
CPasFmt_ThreadList CFmtItem_StdThreadId::m_ThreadList;

/***** CFmtItem_StdThread *****/

class CFmtItem_StdThread : public CFmtItem_Abstract
{
public:
  CFmtItem_StdThread(const std::thread& Value) : m_CvtValue(Value) {}
private:
  const std::thread& m_CvtValue;
public:
  virtual std::string type() override { return "std::thread"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    if (FmtSpec == "t") return CFmtItem_StdThreadId(m_CvtValue.get_id()).str(FmtSpec, Len, Prec);
      else throw ECppFmtTypeException(FmtSpec, "std::thread");
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_StdThread(m_CvtValue);
  }
};

/***** CFmtItem_StdException *****/

class CFmtItem_StdException : public CFmtItem_Abstract
{
public:
  CFmtItem_StdException(const std::exception& Value) : m_CvtValue(Value) {}
private:
  const std::exception& m_CvtValue;
public:
  virtual std::string type() override { return "std::exception"; }
  virtual std::string str(const std::string& FmtSpec, int Len, int Prec) override
  {
    (void)Len;
    (void)Prec;
    if (FmtSpec != "s")throw ECppFmtTypeException(FmtSpec, "exception type");
    std::exception* srcException = const_cast<std::exception*>(&m_CvtValue);
    return std::string(srcException->what());
  }
  virtual CFmtItem_Abstract* clone() const override
  {
    return new CFmtItem_StdException(m_CvtValue);
  }
};

/***** CFmtVariant *****/

CFmtVariant::CFmtVariant(bool Value) : m_Viewer(new CFmtItem_Bool(Value)) {}
CFmtVariant::CFmtVariant(char Value) : m_Viewer(new CFmtItem_Char(Value)) {}
CFmtVariant::CFmtVariant(wchar_t Value) : m_Viewer(new CFmtItem_WChar(Value)) {}
CFmtVariant::CFmtVariant(unsigned char Value) : m_Viewer(new CFmtItem_UChar(Value)) {}
CFmtVariant::CFmtVariant(int Value) : m_Viewer(new CFmtItem_Int(Value)) {}
CFmtVariant::CFmtVariant(short Value) : m_Viewer(new CFmtItem_Int((int)Value)) {}
#ifdef _NATIVE_WCHAR_T_DEFINED //предотвращение ошибок компиляции, связанных с опцией компилятора msvc -Zc:wchar_t-
CFmtVariant::CFmtVariant(unsigned short Value) : m_Viewer(new CFmtItem_Unsigned((unsigned)Value)) {}
#endif // _NATIVE_WCHAR_T_DEFINED
CFmtVariant::CFmtVariant(long Value) : m_Viewer(new CFmtItem_Long(Value)) {}
CFmtVariant::CFmtVariant(long long Value) : m_Viewer(new CFmtItem_LongLong(Value)) {}
CFmtVariant::CFmtVariant(unsigned Value) : m_Viewer(new CFmtItem_Unsigned(Value)) {}
CFmtVariant::CFmtVariant(unsigned long Value) : m_Viewer(new CFmtItem_ULong(Value)) {}
CFmtVariant::CFmtVariant(unsigned long long Value) : m_Viewer(new CFmtItem_ULongLong(Value)) {}
CFmtVariant::CFmtVariant(float Value) : m_Viewer(new CFmtItem_Float(Value)) {}
CFmtVariant::CFmtVariant(double Value) : m_Viewer(new CFmtItem_Double(Value)) {}
CFmtVariant::CFmtVariant(long double Value) : m_Viewer(new CFmtItem_LongDouble(Value)) {}
CFmtVariant::CFmtVariant(char* Value) : m_Viewer(new CFmtItem_PChar(Value)) {}
CFmtVariant::CFmtVariant(const char* Value) : m_Viewer(new CFmtItem_PChar(const_cast<char*>(Value))) {}
CFmtVariant::CFmtVariant(const std::string& Value) : m_Viewer(new CFmtItem_StdString(Value)) {}
CFmtVariant::CFmtVariant(wchar_t* Value) : m_Viewer(new CFmtItem_PWChar(Value)) {}
CFmtVariant::CFmtVariant(const wchar_t* Value) : m_Viewer(new CFmtItem_PWChar(const_cast<wchar_t*>(Value))) {}
CFmtVariant::CFmtVariant(const std::wstring& Value) : m_Viewer(new CFmtItem_StdWString(Value)) {}
CFmtVariant::CFmtVariant(void* Value) : m_Viewer(new CFmtItem_PVoid(Value)) {}
CFmtVariant::CFmtVariant(const void* Value) : m_Viewer(new CFmtItem_PVoid(const_cast<void*>(Value))) {}
CFmtVariant::CFmtVariant(const std::thread::id& Value) : m_Viewer(new CFmtItem_StdThreadId(Value)) {}
CFmtVariant::CFmtVariant(const std::thread& Value) : m_Viewer(new CFmtItem_StdThread(Value)) {}
CFmtVariant::CFmtVariant(const std::exception& Value) : m_Viewer(new CFmtItem_StdException(Value)) {}

std::string CFmtVariant::str(const std::string& FmtSpec, int Len, int Prec) const
{
  std::string result = m_Viewer->str(FmtSpec, Len, Prec);
  return result;
}

/*Создание копии объекта*/
CFmtVariant* CFmtVariant::clone() const
{
  CFmtVariant* result = new CFmtVariant();
  result->m_Viewer = m_Viewer->clone();
  return result;
}

/***** TFmtDictionary *****/

/*Поиск сведений о заданной строке форматированного вывода*/
bool TFmtDictionary::StrFormatExists(const std::string& Str, TFmtCompiledString& Format)
{
  CMutexLocker dictLocker(m_DictAccess);
  auto fmtStrRef = std::find(m_Dictionary.begin(), m_Dictionary.end(), Str);
  bool result = fmtStrRef != m_Dictionary.end();
  if (result) Format = fmtStrRef->format;
  return result;
}

/*Сохранение в словаре сведений о строке форматированного вывода*/
void TFmtDictionary::AddStrFormat(const std::string& Str, const TFmtCompiledString& Format)
{
  CMutexLocker dictLocker(m_DictAccess);
  m_Dictionary.push_back(TFmtStrData(Str, Format));
}

/***** Служебные функции форматирования *****/

std::vector<std::string> strSplit(const std::string& Str, char Delimiter)
{
  std::vector<std::string> result;
  if (Str.empty()) return result;
  std::size_t pos = 0;
  std::size_t len = Str.size();
  for (std::size_t i = 0; i < len; i++) {
    if (Str[i] != Delimiter) continue;
    result.push_back(Str.substr(pos, i - pos));
    pos = i + 1;
  }
  result.push_back((pos == len) ? "" : Str.substr(pos, len));
  return result;
}

int paramCvt(const std::vector<std::string>&FmtParams, std::size_t Idx)
{
  std::size_t checkPos;
  int result = stoi(FmtParams[Idx], &checkPos);
  return (checkPos == FmtParams[Idx].size()) ? result : 0;
}

std::string getFormat(const std::string& FmtSpec, FmtArgs_Type ArgList, std::size_t& ArgIndex)
{
  if (FmtSpec == "") return "";
  std::string fmtLetter = getFormatLetter(FmtSpec);
  if (fmtLetter == "%") return "%";
  std::vector<std::string> fmtParams = strSplit(FmtSpec.substr(1, FmtSpec.size() - fmtLetter.size() - 1), '.');
  for (std::size_t i = 0; i < 2; i++) {
    if (i >= fmtParams.size()) fmtParams.push_back("0");
  }
  int len = paramCvt(fmtParams, 0);
  int prec = paramCvt(fmtParams, 1);
  std::string result = (ArgIndex < ArgList.size()) ? ArgList[ArgIndex++]->str(fmtLetter, len, prec) : "";
  return result;
}

/*Разбиение входной строки на пары [<префикс>;<описатель формата>]*/
void buildFmtList(const std::string& FmtString, TFmtCompiledString& Result, TFmtDictionary* FmtDictionary)
{
  /*Если имеется словарь с данными о строках форматированного вывода, ищем в словаре сделанное ранее разбиение входной строки*/
  if (FmtDictionary != nullptr) {                                         //в случае успешного поиска, найденное разбиение возвращается
    if (FmtDictionary->StrFormatExists(FmtString, Result)) return;        //в качестве результата работы функции
  }
  /*Если словарь отсутствует или в нем нет данных о входной строке, создаем разбиение входной строки*/
  std::regex searchExpr("%(\\d*(\\.\\d*)?)?[lLirw]?[cdDxbfsptTuU%]");
  std::string strConst, strFormat;
  Result.clear();
  for (std::string strSuffix = FmtString; strSuffix != ""; ) {
    std::smatch m;
    if (std::regex_search(strSuffix, m, searchExpr)) {
      strConst = m.prefix();
      strFormat = m.str();
      strSuffix = m.suffix();
    }
    else {
      strConst = strSuffix;
      strFormat = "";
      strSuffix = "";
    }
    Result.push_back(TFmtPair(strConst, strFormat));
  }
  /*При необходимости сохраняем  созданное разбиение входной строки в словаре*/
  if (FmtDictionary != nullptr) {
    FmtDictionary->AddStrFormat(FmtString, Result);
  }
}

/*Получение символьного описателя выводимой величины из строки-полной спецификации формата вывода величины*/
std::string getFormatLetter(const std::string& FmtSpec)
{
  if (FmtSpec == "") return "";
  std::string fmtLetter_1 = FmtSpec.substr(FmtSpec.size() - 1);
  std::string fmtLetter_2 = (FmtSpec.size() == 1) ? "" :
                            (!std::isalpha(FmtSpec[FmtSpec.size()-2], std::locale("C"))) ? "" :
                            std::string(1, FmtSpec[FmtSpec.size()-2]);
  return fmtLetter_2 + fmtLetter_1;
}

/*Получение строки в результате подстановки значений аргументов в исходную строку*/
std::string FormatStr(const std::string& FmtString, FmtArgs_Type ArgList, TFmtDictionary* FmtDictionary)
{
  std::vector<TFmtPair> fmtList;
  buildFmtList(FmtString, fmtList, FmtDictionary);
  std::string result;
  std::size_t argIdx = 0;
  for (const TFmtPair& fmtItem : fmtList) {
    result += fmtItem.first + getFormat(fmtItem.second, ArgList, argIdx);
  }
  return result;
}

/*Получение строки в результате подстановки значений аргументов в исходную строку*/
std::unique_ptr<char[]> FormatChar(const std::string& FmtString, FmtArgs_Type ArgList, TFmtDictionary* FmtDictionary)
{
  std::string str = FormatStr(FmtString, ArgList, FmtDictionary);
  std::unique_ptr<char[]> result(new char[str.size()+1]);
  strcpy(result.get(), str.c_str());
  return result;
}

/***** Вспомогательные функции трассировки работы утилит форматирования и отладчика *****/

//#define DBG_TRACE_WITH_TIME
//#define DBG_TRACE_WITH_THREAD
//#define DBG_TRACE_PATH "C:\\ProgramData\\Skyros\\Logs"

/*Вывод сведений в стандартный файл "dbg_trace.nfo"*/
void _Dbg_Trace(const std::string& Msg)
{
  static std::string openMode = "w+";

  #if defined(DBG_TRACE_WITH_TIME)
  static const std::string outName = FormatStr("dbg_trace_%T.nfo", FmtArgs<<std::time(NULL));
  #elif defined(DBG_TRACE_WITH_THREAD)
  static const std::string outName = FormatStr("dbg_trace_%x.nfo", FmtArgs<<std::this_thread::get_id());
  #else
  static const std::string outName = "dbg_trace.nfo";
  #endif

  #if defined(DBG_TRACE_PATH)
  static const std::string outDir = FormatStr("%s\\", FmtArgs<<DBG_TRACE_PATH);
  #else
  static const std::string outDir = "";
  #endif

  std::string dbgFileName = outDir + outName;
  FILE* outFile = fopen(dbgFileName.c_str(), openMode.c_str());
  std::string outStr = Msg + "\n";
  fputs(outStr.c_str(), outFile);
  fclose(outFile);
  openMode = "a";
}

void _Dbg_Trace(const std::string& Msg, FmtArgs_Type MsgItems)
{
  _Dbg_Trace(FormatStr(Msg, MsgItems));
}
