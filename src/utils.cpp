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

void OrthonormalSystem(const Vector& in, Vector& outRay1, Vector& outRay2)
{
    // is there a reason for these vectors or they are purely random
    const Vector FIXED_SAMPLES[2] = {
            {-0.267261242, 0.534522484, -0.801783726},
            {+0.483368245, 0.096673649, +0.870062840}
    };

    outRay1 = fabs(Dot(in, FIXED_SAMPLES[0])) > 0.99 ? in ^ FIXED_SAMPLES[1] : in ^ FIXED_SAMPLES[0];
    outRay1.Normalize();
    outRay2 = in ^ outRay1;
    outRay2.Normalize();
}

void GenerateDiscPoint(double& outX, double& outY)
{
    double angle = Random() * 2 * PI;
    double radius = sqrt(Random());
    outX = cos(angle) * radius;
    outY = sin(angle) * radius;
}

int ToInt(const std::string& s)
{
    if (s.empty())
        return 0;

    int x;
    if (sscanf(s.c_str(), "%d", &x) == 1)
        return x;

    return 0;
}

double ToDouble(const std::string& s)
{
    if (s.empty())
        return 0;

    double x;
    if (sscanf(s.c_str(), "%lf", &x) == 1)
        return x;

    return 0;
}

std::vector<std::string> Tokenize(const std::string& s)
{
    unsigned i = 0, j, l = s.length();
    std::vector<std::string> result;
    while (i < l)
    {
        while (i < l && isspace(s[i])) i++;
        if (i >= l) break;
        j = i;
        while (j < l && !isspace(s[j])) j++;
        result.push_back(s.substr(i, j - i));
        i = j;
    }
    return result;
}

std::vector<std::string> Split(const std::string& s, char separator)
{
    unsigned i = 0, j, l = s.length();
    std::vector<std::string> result;
    while (i < l)
    {
        j = i;
        while (j < l && s[j] != separator) j++;
        result.push_back(s.substr(i, j - i));
        i = j + 1;
        if (j == l - 1) result.push_back("");
    }

    return result;
}
