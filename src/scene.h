#ifndef RAYTRACING_SCENE_H
#define RAYTRACING_SCENE_H

#include "color.h"
#include "colors.h"
#include "constants.h"
#include "vector.h"

#include <climits>
#include <string>
#include <vector>

enum class ElementType
{
    GEOMETRY,
    SHADER,
    NODE,
    TEXTURE,
    ENVIRONMENT,
    CAMERA,
    SETTINGS,
    LIGHT
};

/// An abstract base class for each element of the scene (Camera, Geometries,...)
/// i.e, anything, that could be described using our scene definition language
class ParsedBlock;
class SceneElement
{
public:
    char name[64] = {0}; //!< A name of this element (a string like "sphere01", "myCamera", etc)
    virtual ~SceneElement() {} //!< a virtual destructor

    virtual ElementType GetElementType() const = 0; //!< Gets the element type

    /**
     * @brief set all the properties of a scene element from a parsed block
     *
     * This is a callback, called by the sceneparser, when it has finished
     * parsing a block of properties, related to some scene element.
     *
     * Consider the following (part of) scene:
     *
     * Sphere mySphere01 {
     *    center (12.5, 0.1, 0.3)
     *    radius 5.0
     * }
     *
     * A class Sphere should inherit from SceneElement and implement fillProperties() in the following manner
     *
     * class Sphere: public SceneElement {
     *	Vector center;
     *	double radius;
     * public:
     * 	void fillProperties(ParsedBlock& pb) {
     *		pb.getVectorProp("center", &center);
     *		pb.getDoubleProp("radius", &radius);
     *	}
     * };
     *
     * (the implementation of SceneElement::fillProperties() does nothing)
     */
    virtual void FillProperties(ParsedBlock& pb) {}

    /**
     * @brief a callback that gets called before the rendering commences
     *
     * If you need to setup some internal data structures before rendering has begun,
     * you should place it here. This callback is executed after scene parsing, and before
     * the rendering commences. You might actually use other SceneElement's, but the beginFrame()
     * function is called for the different classes of SceneElement's in this specific order:
     *
     * 1) Lights
     * 2) Geometries
     * 3) Textures
     * 4) Shaders
     * 5) Nodes
     * 6) Camera
     * 7) GlobalSettings
     *
     * The order of calling beginFrame within the same group is undefined.
     *
     * All these callbacks are called by the Scene::beginRender() function.
     */
    virtual void BeginRender() {}

    /**
     * @brief same as BeginRender(), but gets called before each frame
     *
     * the difference between BeginRender() and BeginFrame() is that BeginRender() is only
     * called once, after parsing is done, whereas BeginFrame is called before every frame
     * (e.g., when rendering an animation).
     */
    virtual void BeginFrame() {}

    friend class SceneParser;
};

struct Node;

class Bitmap;
class Geometry;
class Intersectable;
class SceneParser;
class Shader;
class Texture;
class Transform;
class ParsedBlock
{
public:
    virtual ~ParsedBlock() = default;

    // All these methods are intended to be called by SceneElement implementations
    // each method accepts two parameters: a name, and a value pointer.
    // Each method does one of these things:
    //  - the property with the given name is found and parsed successfully, in which
    //    case the value is filled in, and the method returns true.
    //  - The property is found and it wasn't parsed successfully, in which case
    //    a syntax error exception is raised.
    //  - The property is missing in the scene file, the value is untouched, and the method
    //    returns false (if this is an error, you can signal it with pb.requiredProp(name))
    //
    // Some properties also have min/max ranges. If they are specified and the parsed value
    // does not pass the range check, then a SyntaxError is raised.
    virtual bool GetIntProp(const char* name, int* value, int minValue = INT_MIN, int maxValue = INT_MAX) = 0;
    virtual bool GetUnsignedProp(const char* name, unsigned* value, unsigned maxValue = UINT_MAX) = 0;
    virtual bool GetBoolProp(const char* name, bool* value) = 0;
    virtual bool GetFloatProp(const char* name, float* value, float minValue = -LARGE_FLOAT, float maxValue = LARGE_FLOAT) = 0;
    virtual bool GetDoubleProp(const char* name, double* value, double minValue = -LARGE_DOUBLE, double maxValue = LARGE_DOUBLE) = 0;
    virtual bool GetColorProp(const char* name, Color* value, float minCompValue = -LARGE_FLOAT, float maxCompValue = LARGE_FLOAT) = 0;
    virtual bool GetVectorProp(const char* name, Vector* value) = 0;
    virtual bool GetGeometryProp(const char* name, Geometry** value) = 0;
    virtual bool GetIntersectableProp(const char* name, Intersectable** value) = 0;
    virtual bool GetShaderProp(const char* name, Shader** value) = 0;
    virtual bool GetTextureProp(const char* name, Texture** value) = 0;
    virtual bool GetNodeProp(const char* name, Node** value) = 0;
    virtual bool GetStringProp(const char* name, char* value) = 0; // the buffer should be 256 chars long

    // useful for scene assets like textures, mesh files, etc.
    // the value will hold the full filename to the file.
    // If the file/dir is not found, a FileNotFound exception is raised.
    virtual bool GetFilenameProp(const char* name, char* value) = 0;

    // Does the same logic as GetFilenameProp(), but also loads the bitmap
    // file from the specified file name. The given bitmap is first deleted if not NULL.
    virtual bool GetBitmapFileProp(const char* name, Bitmap& value) = 0;

    // Gets a transform from the parsed block. Namely, it searches for all properties named
    // "scale", "rotate" and "translate" and applies them to T.
    virtual void GetTransformProp(Transform& T) = 0;

    virtual void RequiredProp(const char* name) = 0; // signal an error (missing property of the given name)

    virtual void SignalError(const char* msg) = 0; // signal an error with a specified message
    virtual void SignalWarning(const char* msg) = 0; // signal a warning with a specified message

    // some functions for direct parsed block access:
    virtual int GetBlockLines() = 0;
    virtual void GetBlockLine(int idx, int& srcLine, char head[], char tail[]) = 0;
    virtual SceneParser& GetParser() = 0;
};

class SceneParser
{
public:
    virtual ~SceneParser() {}

    // All these methods are intended to be called by SceneElement implementations:
    virtual Shader* FindShaderByName(const char* name) = 0;
    virtual Texture* FindTextureByName(const char* name) = 0;
    virtual Geometry* FindGeometryByName(const char* name) = 0;
    virtual Node* FindNodeByName(const char* name) = 0;


    /**
     * ResolveFullPath() tries to find a file (or folder), by appending the given path to the directory, where
     * the scene file resides. The idea is that all external files (textures, meshes, etc.) are
     * stored in the same dir where the scene file (*.qdmg) resides, and the paths to that external
     * files do not mention any directories, just the file names.
     *
     * @param path (input-output) - Supply the given file name (as given in the scene file) here.
     *                              If the function succeeds, this will return the full path to a file
     *                              with that name, if it's found.
     * @returns true on success, false on failure (file not found).
     */
    virtual bool ResolveFullPath(char* path) = 0;
};

struct SyntaxError
{
    char msg[128];
    int line;
    SyntaxError();
    SyntaxError(int line, const char* format, ...);
};

struct FileNotFoundError
{
    char filename[245];
    int line;
    FileNotFoundError();
    FileNotFoundError(int line, const char* filename);
};

void StripBracesAndCommas(char *s); //!< strips any braces or commas in a string (in-place)

/// Utility function: gets three doubles from a string in just the same way as the sceneparser will do it
/// (stripping any commas, parentheses, etc)
/// If there is error in parsing, a SyntaxError exception is raised.
void Get3Doubles(int srcLine, char* expression, double& d1, double& d2, double& d3);

/// Splits a string (given in an expression) to a head token and a remaining. The head token is written into
/// `frontToken', and the remaining is copied back to expression
bool GetFrontToken(char* expression, char* frontToken);

/// Splits a string (given in an expression) to a back token and a remaining. The back token is written into
/// `backToken', and the remaining is copied back to expression
bool GetLastToken(char* expression, char* backToken);

void StripPunctuation(char* expression); //!< strips any whitespace or punctuation in front or in the back of a string (in-place)

/// This structure holds all global settings of the scene - frame size, antialiasing toggles, thresholds, etc...
struct GlobalSettings : public SceneElement
{
    int frameWidth = RESX; //!< render window size
    int frameHeight = RESY; //!< render window size

    // Lighting:
    Color ambientLight;                  //!< ambient color

    // AA-related:
    bool wantAA = true;                  //!< is Anti-Aliasing on?
    bool wantAdaptiveAA = true;          //!< apply Anti-Aliasing only when needed
    double aaThreshold = 0.1;            //!< threshold for adaptive Anti-Aliasing
    bool gi;                             //!< is GI on?

    unsigned maxTraceDepth = 4;               //!< maximum recursion depth

    bool dbg = false;                    //!< a debugging flag (if on, various raytracing-related procedures will dump debug info to stdout).
    bool showAA = false;                 //!< will color the Anti-Aliased pixels differently
    Color aaDebugColor = Colors::RED;    //!< the color to be used for showAA;

    bool wantProgressiveDisplay = true;  //!< display progressively the result
    unsigned progressiveDisplayDelay = 100;   //!< the delay the display will be refreshed

    bool useSRGB = false;                //!< whether to use sRGB or RGB

    virtual void FillProperties(ParsedBlock& pb) override;
    virtual ElementType GetElementType() const override { return ElementType::SETTINGS; }
};

class Camera;
class Environment;
struct Light;
struct Scene
{
    std::vector<Geometry*> geometries;
    std::vector<Shader*> shaders;
    std::vector<Node*> nodes;
    std::vector<Node*> superNodes; // also Nodes, but without a shader attached; don't represent an scene object directly
    std::vector<Texture*> textures;
    std::vector<Light*> lights;
    Environment* environment = nullptr;
    Camera* camera = nullptr;
    GlobalSettings settings;

    Scene() = default;
    virtual ~Scene();

    bool ParseScene(const char* sceneFile); //!< Parses a scene file and loads the scene from it. Returns true on success.
    void BeginRender(); //!< Notifies the scene so that a render is about to begin. It calls the BeginRender() method of all scene elements
    void BeginFrame(); //!< Notifies the scene so that a new frame is about to begin. It calls the BeginFrame() method of all scene elements
};

extern Scene scene;

#endif //RAYTRACING_SCENE_H
