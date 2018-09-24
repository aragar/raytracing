#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>

std::string UpCaseString(std::string s)
{
    for (int i = 0; i < (int) s.length(); i++)
        s[i] = toupper(s[i]);
    return s;
}
std::string ExtensionUpper(const char* filename)
{
    int filenameLength = (int) strlen(filename);
    if (filenameLength < 2)
        return "";

    for (int i = filenameLength - 1; i >= 0; i--)
    {
        if (filename[i] == '.')
        {
            std::string result = "";
            for  (int j = i + 1; j < filenameLength; j++)
                result += toupper(filename[j]);

            return result;
        }
    }

    return "";
}