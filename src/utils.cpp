#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"

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

bool FileExists(const char* filename)
{
    char temp[512];
    strcpy(temp, filename);
    int filenameLength = (int) strlen(temp);
    if (filenameLength && temp[filenameLength - 1] == '/')
        temp[--filenameLength] = 0;

    struct stat st;
    return (0 == stat(temp, &st));
}

void OrthonormalSystem(const Vector& in, Vector& ray1, Vector& ray2)
{
    // is there a reason for these vectors or they are purely random
    const Vector FIXED_SAMPLES[2] = {
            {-0.267261242, 0.534522484, -0.801783726},
            {+0.483368245, 0.096673649, +0.870062840}
    };

    ray1 = fabs(Dot(in, FIXED_SAMPLES[0])) > 0.99 ? in ^ FIXED_SAMPLES[1] : in ^ FIXED_SAMPLES[0];
    ray1.Normalize();
    ray2 = in ^ ray1;
    ray2.Normalize();
}
