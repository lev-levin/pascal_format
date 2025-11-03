# pascal_format

Library "pascal_format" is a static library on C++11 language, implementing a single function "FormatStr".
This function works very similar to the Format function from Borland Delphi. In other words, it takes 2 parameters: a string template with format specifiers and a set of substituted values, represented as a separate object; and returns
the string, resulting from substituting values into the template.  
Why is it convenient to use FormatStr instead of sprintf?:  
1. The entire set of substituted values is a single parameter, i.e. it can be represented as an "FmtArgs_Var" type variable and, further, used repeatedly or passed as a parameter, for example, to a logging function
2. The ability to connect your own data types to the set of substituted values, which significantly expands the scope of the function (for example: converting enum values to words, instead digits)
3. And finally, the return value is: std::string. No need to be distracted by additional manipulations with the output buffer.  
The "example" subdirectory contains a small example of using the "FormatStr" function.  
  
Description of the standard format specifiers of the FormatStr function:  
%c - character  
%d - integer (int;long;long long) in decimal format (leading characters are zeros)  
     unsigned integer (uint;ulong;ulong long) in decimal format (leading characters are zeros)  
%D - integer (int;long;long long) in decimal format (leading characters are spaces)  
     unsigned integer (uint;ulong;ulong long) in decimal format (leading characters are spaces)  
%ld - long integer (long) in decimal format (leading characters are zeros)  
%lD - long integer (long) in decimal format (leading characters are spaces)  
%Ld - 64-bit integer (long long) in decimal format (leading characters are zeros)  
%LD - 64-bit integer (ulong long) in decimal format (leading characters are spaces)  
%u - unsigned integer (uint) in decimal format (leading characters are zeros)  
%U - unsigned integer (uint) in decimal format (leading characters are spaces)  
%lu - unsigned long integer (ulong) in decimal format (leading characters are zeros)  
%lU - unsigned long integer (ulong) in decimal format (leading characters are spaces)  
%Lu - unsigned 64-bit integer (ulong long) in decimal format (leading characters are zeros)  
%LU - unsigned 64-bit integer (ulong long) in decimal format (leading characters are spaces)  
%x - character in 16-bit form (for example: AF)  
     unsigned integer in 16-bit form  
     C-string in 16-bit format (the Len parameter must specify the length of the string or 0 - to converting before the character '\0';  
       the Prec parameter must specify the number of bytes of the string to display, or 0 - Len bytes)  
     a std::string in 16-bit format (the Len parameter must specify the length of the string or 0 - the entire string;  
       the Prec parameter must specify the number of bytes of the string to display, or 0 - Len bytes)  
%lx - unsigned long integer (ulong) in 16-bit format  
%Lx - unsigned 64-bit integer (ulong long) in 16-bit format  
%b - unsigned integer in the form of a bit string.  
%lb - unsigned long integer (ulong) in the form of a bit string.  
%Lb - unsigned 64-bit integer (ulong long) in the form of a bit string.  
%f - floating point number (float;double) in decimal format.  
%lf - floating point number (double) in decimal format  
%Lf - floating point number (long double) in decimal format  
%s - C-string in the form of a string  
     std::string as a string  
     boolean value as a string (for integer types)  
     boolean value "not NULL" (for pointer)  
     description of the std::exception as a string  
%is - logical value in the form of a string (for int type: not zero/zero)  
%ls - logical value in the form of a string (for long type: not zero/zero)  
%Ls - logical value in the form of a string (for long type long: not zero/zero)  
%rs - logical value as a string (for void* type: not NULL/NULL)  
%p - pointer  
%t - stream ID  
%T - time in the hh-mm-ss format (for the long long type aka time_t)  
  
For some format specifiers, you can use the notation %Len.Prec<specifier>, where Len is the number of characters to output; Prec is the number of decimal places.  
