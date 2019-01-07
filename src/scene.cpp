#include "scene.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "bitmap.h"
#include "camera.h"
#include "environment.h"
#include "geometry.h"
#include "light.h"
#include "mesh.h"
#include "random_generator.h"
#include "sdl.h"
#include "shading.h"
#include "texture.h"
#include "transform.h"
#include "utils.h"

class DefaultSceneParser;
class ParsedBlockImpl: public ParsedBlock
{
public:
    virtual bool GetIntProp(const char* name, int* value, int minValue = INT_MIN, int maxValue = INT_MAX) override;
    virtual bool GetUnsignedProp(const char* name, unsigned* value, unsigned maxValue = UINT_MAX) override;
    virtual bool GetBoolProp(const char* name, bool* value) override;
    virtual bool GetFloatProp(const char* name, float* value, float minValue = -LARGE_FLOAT, float maxValue = LARGE_FLOAT) override;
    virtual bool GetDoubleProp(const char* name, double* value, double minValue = -LARGE_DOUBLE, double maxValue = LARGE_DOUBLE) override;
    virtual bool GetColorProp(const char* name, Color* value, float minCompValue = -LARGE_FLOAT, float maxCompValue = LARGE_FLOAT) override;
    virtual bool GetVectorProp(const char* name, Vector* value) override;
    virtual bool GetGeometryProp(const char* name, Geometry** value) override;
    virtual bool GetIntersectableProp(const char* name, Intersectable** value) override;
    virtual bool GetShaderProp(const char* name, Shader** value) override;
    virtual bool GetTextureProp(const char* name, Texture** value) override;
    virtual bool GetNodeProp(const char* name, Node** value) override;
    virtual bool GetStringProp(const char* name, char* value) override;
    virtual bool GetFilenameProp(const char* name, char* value) override;
    virtual bool GetBitmapFileProp(const char* name, Bitmap& value) override;
    virtual void GetTransformProp(Transform& T) override;
    virtual void RequiredProp(const char* name) override;
    virtual void SignalError(const char* msg) override;
    virtual void SignalWarning(const char* msg) override;
    virtual int GetBlockLines() override;
    virtual void GetBlockLine(int idx, int& srcLine, char head[], char tail[]) override;
    virtual SceneParser& GetParser() override;

private:
    friend class DefaultSceneParser;
    struct LineInfo
    {
        int line;
        char propName[128];
        char propValue[256];
        bool recognized;

        LineInfo() {}
        LineInfo(int line, const char* name, const char* value): line(line)
        {
            strncpy(propName, name, sizeof(propName));
            strncpy(propValue, value, sizeof(propValue));
            recognized = false;
        }
    };

    std::vector<LineInfo> m_Lines;
    int m_BlockLine, m_BlockEnd; // line numbers
    SceneParser* m_Parser;
    SceneElement* m_Element;

    bool FindProperty(const char* name, int& i_s, int& line_s, char*& value);
};



SyntaxError::SyntaxError()
{
    line = -1;
    msg[0] = 0;
}

SyntaxError::SyntaxError(int line, const char* format, ...)
{
    this->line = line;
    va_list ap;
    va_start(ap, format);

#ifdef _MSC_VER
#	define vsnprintf _vsnprintf
#endif

    vsnprintf(msg, sizeof(msg), format, ap);
    va_end(ap);
}

FileNotFoundError::FileNotFoundError() {}
FileNotFoundError::FileNotFoundError(int line, const char* filename)
{
    this->line = line;
    strcpy(this->filename, filename);
}

bool ParsedBlockImpl::FindProperty(const char* name, int& i_s, int& line_s, char*& value)
{
    for (int i = 0; i < (int) m_Lines.size(); i++)
    {
        if (!strcmp(m_Lines[i].propName, name))
        {
            i_s = i;
            line_s = m_Lines[i].line;
            value = m_Lines[i].propValue;
            m_Lines[i].recognized = true;
            return true;
        }
    }

    return false;
}

#define PBEGIN\
	int i_s = 0; \
	char* value_s; \
	int line = 0; \
	if (!FindProperty(name, i_s, line, value_s)) return false;

bool ParsedBlockImpl::GetIntProp(const char* name, int* value, int minValue, int maxValue)
{
    PBEGIN;
    int x;
    if (1 != sscanf(value_s, "%d", &x)) throw SyntaxError(line, "Invalid integer");
    if (x < minValue || x > maxValue) throw SyntaxError(line, "Value outside the allowed bounds (%d .. %d)\n", minValue, maxValue);
    *value = x;
    return true;
}

bool ParsedBlockImpl::GetUnsignedProp(const char* name, unsigned* value, unsigned int maxValue)
{
    PBEGIN;
    unsigned x;
    if (1 != sscanf(value_s, "%u", &x)) throw SyntaxError(line, "Invalid unsigned");
    if (x > maxValue) throw SyntaxError(line, "Value is bigger than the allowed max (%u)\n", maxValue);
    *value = x;
    return true;
}


bool ParsedBlockImpl::GetBoolProp(const char* name, bool* value)
{
    PBEGIN;
    *value = strcmp(value_s, "off") && strcmp(value_s, "false") && strcmp(value_s, "0");
    return true;
}

bool ParsedBlockImpl::GetFloatProp(const char* name, float* value, float minValue, float maxValue)
{
    PBEGIN;
    float x;
    if (1 != sscanf(value_s, "%f", &x)) throw SyntaxError(line, "Invalid float");
    if (x < minValue || x > maxValue) throw SyntaxError(line, "Value outside the allowed bounds (%f .. %f)\n", minValue, maxValue);
    *value = x;
    return true;
}

bool ParsedBlockImpl::GetDoubleProp(const char* name, double* value, double minValue, double maxValue)
{
    PBEGIN;
    double x;
    if (1 != sscanf(value_s, "%lf", &x)) throw SyntaxError(line, "Invalid double");
    if (x < minValue || x > maxValue) throw SyntaxError(line, "Value outside the allowed bounds (%f .. %f)\n", minValue, maxValue);
    *value = x;
    return true;
}

void StripBracesAndCommas(char *s)
{
    int l = (int) strlen(s);
    for (int i = 0; i < l; i++)
    {
        char& c = s[i];
        if (c == ',' || c == '(' || c == ')') c = ' ';
    }
}

bool ParsedBlockImpl::GetColorProp(const char* name, Color* value, float minCompValue, float maxCompValue)
{
    PBEGIN;
    StripBracesAndCommas(value_s);
    Color c;
    if (3 != sscanf(value_s, "%f%f%f", &c.r, &c.g, &c.b)) throw SyntaxError(line, "Invalid color");
    if (c.r < minCompValue || c.r > maxCompValue) throw SyntaxError(line, "Color R value outside the allowed bounds (%f .. %f)\n", minCompValue, maxCompValue);
    if (c.g < minCompValue || c.g > maxCompValue) throw SyntaxError(line, "Color G value outside the allowed bounds (%f .. %f)\n", minCompValue, maxCompValue);
    if (c.b < minCompValue || c.b > maxCompValue) throw SyntaxError(line, "Color B value outside the allowed bounds (%f .. %f)\n", minCompValue, maxCompValue);
    *value = c;
    return true;
}

bool ParsedBlockImpl::GetVectorProp(const char* name, Vector* value)
{
    PBEGIN;
    StripBracesAndCommas(value_s);
    Vector v;
    if (3 != sscanf(value_s, "%lf%lf%lf", &v.x, &v.y, &v.z)) throw SyntaxError(line, "Invalid vector");
    *value = v;
    return true;
}

bool ParsedBlockImpl::GetGeometryProp(const char* name, Geometry** value)
{
    PBEGIN;
    Geometry* g = m_Parser->FindGeometryByName(value_s);
    if (!g) throw SyntaxError(line, "Geometry not defined");
    *value = g;
    return true;
}

bool ParsedBlockImpl::GetIntersectableProp(const char* name, Intersectable** value)
{
    PBEGIN;
    if (Geometry* g = m_Parser->FindGeometryByName(value_s))
    {
        *value = g;
        return true;
    }
    Node* node = m_Parser->FindNodeByName(value_s);
    if (!node) throw SyntaxError(line, "Intersectable by that name not defined");
    *value = static_cast<Intersectable*>(node);
    return true;
}

bool ParsedBlockImpl::GetShaderProp(const char* name, Shader** value)
{
    PBEGIN;
    Shader* s = m_Parser->FindShaderByName(value_s);
    if (!s) throw SyntaxError(line, "Shader not defined");
    *value = s;
    return true;
}

bool ParsedBlockImpl::GetTextureProp(const char* name, Texture** value)
{
    PBEGIN;
    Texture* t = m_Parser->FindTextureByName(value_s);
    if (!t) throw SyntaxError(line, "Texture not defined");
    *value = t;
    return true;
}

bool ParsedBlockImpl::GetNodeProp(const char* name, Node** value)
{
    PBEGIN;
    Node* n = m_Parser->FindNodeByName(value_s);
    if (!n) throw SyntaxError(line, "Node not defined");
    *value = n;
    return true;
}

bool ParsedBlockImpl::GetStringProp(const char* name, char* value)
{
    PBEGIN;
    strcpy(value, value_s);
    return true;
}

bool ParsedBlockImpl::GetFilenameProp(const char* name, char* value)
{
    PBEGIN;
    strcpy(value, value_s);
    if (m_Parser->ResolveFullPath(value)) return true;
    else throw FileNotFoundError(line, value_s);
}

bool ParsedBlockImpl::GetBitmapFileProp(const char* name, Bitmap& bmp)
{
    PBEGIN;
    char filename[256];
    strcpy(filename, value_s);
    if (!m_Parser->ResolveFullPath(filename)) throw FileNotFoundError(line, filename);
    return bmp.LoadImage(filename);
}

void ParsedBlockImpl::GetTransformProp(Transform& T)
{
    for (int i = 0; i < (int) m_Lines.size(); i++)
    {
        double x, y, z;
        if (!strcmp(m_Lines[i].propName, "scale"))
        {
            m_Lines[i].recognized = true;
            Get3Doubles(m_Lines[i].line, m_Lines[i].propValue, x, y, z);
            T.Scale(x, y, z);
            continue;
        }
        if (!strcmp(m_Lines[i].propName, "rotate"))
        {
            m_Lines[i].recognized = true;
            Get3Doubles(m_Lines[i].line, m_Lines[i].propValue, x, y, z);
            T.Rotate(x, y, z);
            continue;
        }
        if (!strcmp(m_Lines[i].propName, "translate"))
        {
            m_Lines[i].recognized = true;
            Get3Doubles(m_Lines[i].line, m_Lines[i].propValue, x, y, z);
            T.Translate(Vector(x, y, z));
            continue;
        }
    }
}

void ParsedBlockImpl::RequiredProp(const char* name)
{
    int k1, k2;
    char* value;
    if (!FindProperty(name, k1, k2, value))
    {
        throw SyntaxError(m_BlockEnd, "Required property `%s' not defined", name);
    }
}

void ParsedBlockImpl::SignalError(const char* msg)
{
    throw SyntaxError(m_BlockEnd, msg);
}

void ParsedBlockImpl::SignalWarning(const char* msg)
{
    fprintf(stderr, "Warning (at line %d): %s\n", m_BlockEnd, msg);
}

int ParsedBlockImpl::GetBlockLines()
{
    return (int) m_Lines.size();
}

void ParsedBlockImpl::GetBlockLine(int idx, int& srcLine, char head[], char tail[])
{
    m_Lines[idx].recognized = true;
    srcLine = m_Lines[idx].line;
    strcpy(head, m_Lines[idx].propName);
    strcpy(tail, m_Lines[idx].propValue);
}

SceneParser& ParsedBlockImpl::GetParser()
{
    return *m_Parser;
}

class DefaultSceneParser: public SceneParser
{
private:
    char m_SceneRootDir[256] = {0};
    Scene* m_S = nullptr;
    SceneElement* m_CurObj = nullptr;
    int m_CurLine;
    void ReplaceRandomNumbers(int srcLine, char* line, class Random& rnd);

public:
    ~DefaultSceneParser() override {};

    SceneElement* NewSceneElement(const char* className);
    bool ResolveFullPath(char* path) override;

    Shader* FindShaderByName(const char* name) override;
    Texture* FindTextureByName(const char* name) override;
    Geometry* FindGeometryByName(const char* name) override;
    Node* FindNodeByName(const char* name) override;

    bool Parse(const char* filename, Scene* s);
};

static void StripWhiteSpace(char* s)
{
    int i = (int) strlen(s) - 1;
    while (i >= 0 && isspace(s[i])) i--;
    s[++i] = 0;
    int l = i;
    i = 0;
    while (i < l && isspace(s[i])) i++;
    if (i > 0 && i < l)
    {
        for (int j = 0; j <= l - i; j++)
            s[j] = s[i + j];
    }
}

bool DefaultSceneParser::Parse(const char* filename, Scene* ss)
{
    class Random& rnd = GetRandomGen(0);
    m_S = ss;
    m_CurObj = nullptr;
    m_CurLine = 0;
    m_S->environment = nullptr;
    //
    FILE* f = fopen(filename, "rt");
    if (!f)
    {
        fprintf(stderr, "Cannot open scene file `%s'!\n", filename);
        return false;
    }

    if (unsigned i = strlen(filename))
    {
        m_SceneRootDir[0] = 0;
        while (i > 0 && (filename[i - 1] != '/' && filename[i - 1] != '\\')) i--;
        if ( i > 0 )
        {
            strncpy(m_SceneRootDir, filename, i);
            m_SceneRootDir[i] = 0;
        }
    }

    FileRAII fraii(f);
    char line[1024];
    bool commentedOut = false;
    std::vector<ParsedBlockImpl> parsedBlocks;
    ParsedBlockImpl* cblock = nullptr;
    while (fgets(line, sizeof(line), f))
    {
        m_CurLine++;
        if (commentedOut)
        {
            if (line[0] == '*' && line[1] == '/') commentedOut = false;
            continue;
        }

        char *commentBegin = nullptr;
        if (strstr(line, "//")) commentBegin = strstr(line, "//");

        if (strstr(line, "#"))
        {
            char* temp = strstr(line, "#");
            if (!commentBegin || commentBegin > temp) commentBegin = temp;
        }

        if (commentBegin) *commentBegin = 0;

        StripWhiteSpace(line);
        if (strlen(line) == 0) continue; // empty line
        if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) continue; // comment
        if (line[0] == '/' && line[1] == '*')
        {
            commentedOut = true;
            continue;
        }

        ReplaceRandomNumbers(m_CurLine, line, rnd);
        std::vector<std::string> tokens = Tokenize(line);
        if (!m_CurObj)
        {
            switch (tokens.size())
            {
                case 1:
                {
                    if (tokens[0] == "{")
                        fprintf(stderr, "Excess `}' on line %d\n", m_CurLine);
                    else
                        fprintf(stderr, "Unexpected token `%s' on line %d\n", tokens[0].c_str(), m_CurLine);
                    return false;
                }
                case 2:
                {
                    if (tokens[1] != "{")
                    {
                        fprintf(stderr, "A singleton object definition should end with a `{' (on line %d)\n", m_CurLine);
                        return false;
                    }
                    m_CurObj = NewSceneElement(tokens[0].c_str());
                    if (m_CurObj) m_CurObj->name[0] = 0;
                    break;
                }
                case 3:
                {
                    if (tokens[2] != "{")
                    {
                        fprintf(stderr, "A object definition should end with a `{' (on line %d)\n", m_CurLine);
                        return false;
                    }
                    m_CurObj = NewSceneElement(tokens[0].c_str());
                    break;
                }
                default:
                {
                    fprintf(stderr, "Unexpected content on line %d!\n", m_CurLine);
                    return false;
                }
            }

            if (m_CurObj)
            {
                strcpy(m_CurObj->name, tokens[1].c_str());
                parsedBlocks.push_back(ParsedBlockImpl());
                cblock = &parsedBlocks[parsedBlocks.size() - 1];
                cblock->m_Parser = this;
                cblock->m_Element = m_CurObj;
                cblock->m_BlockLine = m_CurLine;
            }
            else
            {
                fprintf(stderr, "Unknown object class `%s' on line %d\n", tokens[0].c_str(), m_CurLine);
                return false;
            }

            ElementType et = m_CurObj->GetElementType();
            switch (et)
            {
                case ElementType::GEOMETRY:     m_S->geometries.push_back((Geometry*)m_CurObj); break;
                case ElementType::SHADER:       m_S->shaders.push_back((Shader*)m_CurObj); break;
                case ElementType::TEXTURE:      m_S->textures.push_back((Texture*)m_CurObj); break;
                case ElementType::NODE:         m_S->nodes.push_back((Node*)m_CurObj); break;
                case ElementType::ENVIRONMENT:  m_S->environment = (Environment*) m_CurObj; break;
                case ElementType::CAMERA:       m_S->camera = (Camera*)m_CurObj; break;
                case ElementType::LIGHT:        m_S->lights.push_back((Light*)m_CurObj); break;
                default: break;
            }
        }
        else
        {
            if (tokens.size() == 1)
            {
                if (tokens[0] == "}")
                {
                    cblock->m_BlockEnd = m_CurLine;
                    m_CurObj = NULL;
                    cblock = NULL;
                }
                else
                {
                    fprintf(stderr, "Unexpected token in object definition on line %d: `%s'\n", m_CurLine, tokens[0].c_str());
                    return false;
                }
            }
            else
            {
                // try to find a property with that name...
                int i = (int) tokens[0].length();
                while (isspace(line[i])) i++;
                int l = (int) strlen(line) - 1;
                if (i < l && line[i] == '"' && line[l] == '"')
                { // strip the quotes of a quoted argument
                    line[l] = 0;
                    i++;
                }

                const ParsedBlockImpl::LineInfo& lineInfo = ParsedBlockImpl::LineInfo(m_CurLine, tokens[0].c_str(), line + i);
                cblock->m_Lines.push_back(lineInfo);
            }
        }
    }

    if (m_CurObj)
    {
        fprintf(stderr, "Unfinished object definition at EOF!\n");
        return false;
    }

    const ElementType element_types_order[] = {
            ElementType::SETTINGS,
            ElementType::CAMERA,
            ElementType::ENVIRONMENT,
            ElementType::LIGHT,
            ElementType::GEOMETRY,
            ElementType::TEXTURE,
            ElementType::SHADER,
            ElementType::NODE,
            //ELEM_ATMOSPHERIC
    };
    // process all parsed blocks, but first process all singletons, then process geometries first, etc.
    for (ElementType elementType : element_types_order)
    {
        for (ParsedBlockImpl& pb : parsedBlocks)
        {
            if (pb.m_Element->GetElementType() == elementType)
            {
                try
                {
                    pb.m_Element->FillProperties(pb);
                }
                catch (SyntaxError err)
                {
                    fprintf(stderr, "%s:%d: Syntax error on line %d: %s\n", filename, err.line, err.line, err.msg);
                    return false;
                }
                catch (FileNotFoundError err)
                {
                    fprintf(stderr, "%s:%d: Required file not found (%s) (required at line %d)\n", filename, err.line, err.filename, err.line);
                    return false;
                }

                for (ParsedBlockImpl::LineInfo& lineInfo : pb.m_Lines)
                    if (!lineInfo.recognized)
                        fprintf(stderr, "%s:%d: Warning: the property `%s' isn't recognized!\n", filename, lineInfo.line, lineInfo.propName);
            }
        }
    }

    // filter out the nodes[] array; any nodes, which don't have a shader attached are transferred to the
    // subnodes array:
    for (unsigned i = m_S->nodes.size(); i > 0; i--)
    {
        if (!m_S->nodes[i - 1]->shader)
        {
            m_S->superNodes.push_back(m_S->nodes[i - 1]);
            m_S->nodes.erase(m_S->nodes.begin() + i - 1);
        }
    }
    return true;
}

Shader* DefaultSceneParser::FindShaderByName(const char* name)
{
    for (auto& shader: m_S->shaders) {
        if (!strcmp(shader->name, name)) {
            return shader;
        }
    }
    return nullptr;
}
Geometry* DefaultSceneParser::FindGeometryByName(const char* name)
{
    for (auto& geom: m_S->geometries) {
        if (!strcmp(geom->name, name)) {
            return geom;
        }
    }
    return nullptr;
}
Texture* DefaultSceneParser::FindTextureByName(const char* name)
{
    for (auto& tex: m_S->textures)
    {
        if (!strcmp(tex->name, name))
        {
            return tex;
        }
    }
    return nullptr;
}
Node* DefaultSceneParser::FindNodeByName(const char* name)
{
    for (auto& node: m_S->nodes)
    {
        if (!strcmp(node->name, name))
        {
            return node;
        }
    }
    return nullptr;
}

void DefaultSceneParser::ReplaceRandomNumbers(int srcLine, char* s, class Random& rnd)
{
    while (strstr(s, "randfloat"))
    {
        char* p = strstr(s, "randfloat");
        int i, j;
        for (i = 0; p[i] && p[i] != '('; i++);
        if (p[i] != '(') throw SyntaxError(srcLine, "randfloat in inexpected format");
        for (j = i; p[j] && p[j] != ')'; j++);
        if (p[j] != ')') throw SyntaxError(srcLine, "randfloat in inexpected format");
        p[j] = 0;
        float f1, f2;
        if (2 != sscanf(p + i + 1, "%f,%f", &f1, &f2)) throw SyntaxError(srcLine, "bad randfloat format (expected: randfloat(<min>, <max>))");
        if (f1 > f2) throw SyntaxError(srcLine, "bad randfloat format (min > max)");
        float res = rnd.RandFloat() * (f2 - f1) + f1;
        for (int k = 0; k <= j; k++)
            p[k] = ' ';
        char temp[30];
        sprintf(temp, "%.5f", res);
        int l = (int) strlen(temp);
        assert (l < j);
        for (int i = 0; i < l; i++)
            p[i] = temp[i];
    }
    while (strstr(s, "randint"))
    {
        char* p = strstr(s, "randint");
        int i, j;
        for (i = 0; p[i] && p[i] != '('; i++);
        if (p[i] != '(') throw SyntaxError(srcLine, "randint in inexpected format");
        for (j = i; p[j] && p[j] != ')'; j++);
        if (p[j] != ')') throw SyntaxError(srcLine, "randint in inexpected format");
        p[j] = 0;
        int f1, f2;
        if (2 != sscanf(p + i + 1, "%d,%d", &f1, &f2)) throw SyntaxError(srcLine, "bad randint format (expected: randint(<min>, <max>))");
        if (f1 > f2) throw SyntaxError(srcLine, "bad randint format (min > max)");
        int res = rnd.RandInt(f1, f2);
        for (int k = 0; k <= j; k++)
            p[k] = ' ';
        char temp[30];
        sprintf(temp, "%d", res);
        int l = (int) strlen(temp);
        assert (l < j);
        for (int i = 0; i < l; i++)
            p[i] = temp[i];
    }
}

void Get3Doubles(int srcLine, char* expression, double& d1, double& d2, double& d3)
{
    int l = (int) strlen(expression);
    for (int i = 0; i < l; i++)
    {
        char c = expression[i];
        if (c == '(' || c == ')' || c == ',') expression[i] = ' ';
    }
    if (3 != sscanf(expression, "%lf%lf%lf", &d1, &d2, &d3))
    {
        throw SyntaxError(srcLine, "Expected three double values");
    }
}

bool GetFrontToken(char* s, char* frontToken)
{
    int l = (int) strlen(s);
    int i = 0;
    while (i < l && isspace(s[i])) i++;
    if (i == l) return false;
    int j = 0;
    while (i < l && !isspace(s[i])) frontToken[j++] = s[i++];
    if (i == l) return false;
    frontToken[j] = 0;
    j = 0;
    while (i <= l) s[j++] = s[i++];
    return true;
}

bool GetLastToken(char* s, char* backToken)
{
    int l = (int) strlen(s);
    int i = l - 1;
    while (i >= 0 && isspace(s[i])) i--;
    if (i < 0) return false;
    int j = i;
    while (j >= 0 && !isspace(s[j])) j--;
    if (j < 0) return false;
    strncpy(backToken, &s[j + 1], i - j);
    backToken[i - j] = 0;
    s[++j] = 0;
    return true;
}

void StripPunctuation(char* s)
{
    char temp[1024];
    strncpy(temp, s, sizeof(temp));
    int l = (int) strlen(temp);
    int j = 0;
    for (int i = 0; i < l; i++)
    {
        if (!isspace(temp[i]) && temp[i] != ',')
            s[j++] = temp[i];
    }
    s[j] = 0;
}

bool DefaultSceneParser::ResolveFullPath(char* path)
{
    char temp[256];
    strcpy(temp, m_SceneRootDir);
    strcat(temp, path);
    if (FileExists(temp))
    {
        strcpy(path, temp);
        return true;
    }
    else
    {
        return false;
    }
}

template<typename T>
void DisposeArray(std::vector<T>& objects)
{
    for (auto& object: objects)
        if (object)
            delete object;

    objects.clear();
}

Scene::~Scene()
{
    DisposeArray(geometries);
    DisposeArray(nodes);
    DisposeArray(superNodes);
    DisposeArray(textures);
    DisposeArray(shaders);
    DisposeArray(lights);
    if (environment)
        delete environment;
    environment = nullptr;
    if (camera)
        delete camera;
    camera = nullptr;
}

bool Scene::ParseScene(const char* filename)
{
    DefaultSceneParser parser;
    return parser.Parse(filename, this);
}

void Scene::BeginRender()
{
    for (auto& element: geometries) element->BeginRender();
    for (auto& element: textures) element->BeginRender();
    for (auto& element: shaders) element->BeginRender();
    for (auto& element: superNodes) element->BeginRender();
    for (auto& element: nodes) element->BeginRender();
    for (auto& element: lights) element->BeginRender();
    camera->BeginRender();
    settings.BeginRender();
    if (environment)
        environment->BeginRender();
}

void Scene::BeginFrame()
{
    for (auto& element: geometries) element->BeginFrame();
    for (auto& element: textures) element->BeginFrame();
    for (auto& element: shaders) element->BeginFrame();
    for (auto& element: superNodes) element->BeginFrame();
    for (auto& element: nodes) element->BeginFrame();
    for (auto& element: lights) element->BeginFrame();
    camera->BeginFrame();
    settings.BeginFrame();
    if (environment)
        environment->BeginFrame();
}

void GlobalSettings::FillProperties(ParsedBlock& pb)
{
    pb.GetIntProp("frameWidth", &frameWidth);
    pb.GetIntProp("frameHeight", &frameHeight);

    pb.GetColorProp("ambientLight", &ambientLight);

    pb.GetBoolProp("wantAA", &wantAA);
    pb.GetBoolProp("wantAdaptiveAA", &wantAdaptiveAA);
    pb.GetDoubleProp("aaThreshold", &aaThreshold);

    pb.GetUnsignedProp("maxTraceDepth", &maxTraceDepth);

    pb.GetBoolProp("dbg", &dbg);
    pb.GetBoolProp("showAA", &showAA);
    pb.GetColorProp("aaDebugColor", &aaDebugColor);

    pb.GetBoolProp("wantProgressiveDisplay", &wantProgressiveDisplay);
    pb.GetUnsignedProp("progressiveDisplayDelay", &progressiveDisplayDelay);

    pb.GetBoolProp("useSRGB", &useSRGB);
}

SceneElement* DefaultSceneParser::NewSceneElement(const char* className)
{
    if (!strcmp(className, "GlobalSettings")) return &m_S->settings;

    // geometries
    if (!strcmp(className, "Plane")) return new Plane;
    if (!strcmp(className, "RegularPolygon")) return new RegularPolygon;
    if (!strcmp(className, "Sphere")) return new Sphere;
    if (!strcmp(className, "Cube")) return new Cube;
    if (!strcmp(className, "CsgPlus")) return new CsgPlus;
    if (!strcmp(className, "CsgAnd")) return new CsgAnd;
    if (!strcmp(className, "CsgMinus")) return new CsgMinus;
    if (!strcmp(className, "Node")) return new Node;
    if (!strcmp(className, "Mesh")) return new Mesh;

    // shaders
    if (!strcmp(className, "Lambert")) return new Lambert;
    if (!strcmp(className, "Phong")) return new Phong;
    if (!strcmp(className, "BlinnPhong")) return new BlinnPhong;
    if (!strcmp(className, "OrenNayar")) return new OrenNayar;
    if (!strcmp(className, "Reflection")) return new Reflection;
    if (!strcmp(className, "Refraction")) return new Refraction;
    if (!strcmp(className, "Layered")) return new Layered;

    // textures
    if (!strcmp(className, "ConstantColorTexture")) return new ConstantColorTexture;
    if (!strcmp(className, "CheckerTexture")) return new CheckerTexture;
    if (!strcmp(className, "MandelbrotTexture")) return new MandelbrotTexture;
    if (!strcmp(className, "ProceduralTexture")) return new ProceduralTexture;
    if (!strcmp(className, "BitmapTexture")) return new BitmapTexture;
    if (!strcmp(className, "Fresnel")) return new Fresnel;
    if (!strcmp(className, "BumpTexture")) return new BumpTexture;

    // environment
    if (!strcmp(className, "CubemapEnvironment")) return new CubemapEnvironment;

    // camera
    if (!strcmp(className, "Camera")) return new Camera;

    // light
    if (!strcmp(className, "Light")) return new Light;

    return nullptr;
}

Scene scene;