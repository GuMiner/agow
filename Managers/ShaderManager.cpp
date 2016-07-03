#include <sstream>
#include "Utils\Logger.h"
#include "Utils\StringUtils.h"
#include "ShaderManager.h"

ShaderManager::ShaderManager()
    : shaderPrograms()
{
}

bool ShaderManager::ReadShader(const char *rootName, const char *extension, std::string* readShader)
{
    std::stringstream filenameStream;
    filenameStream << "shaders/" << rootName << extension;

    if (!StringUtils::LoadStringFromFile(filenameStream.str().c_str(), *readShader))
    {
        Logger::LogError("Could not load ", extension, " shader: ", *readShader, "!");
        return false;
    }

    return true;
}

// Creates and compiles a new shader of the specified type; returns true on success.
bool ShaderManager::CreateShader(GLenum shaderType, const char *shaderSource, GLuint *shaderId)
{
    GLint compileStatus;
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus)
    {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::stringstream logStream;
        char *shaderLog = new char[logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, &shaderLog[0]);
        logStream << "GLSL compilation error: " << shaderLog << ".";
        
        Logger::LogError(logStream.str().c_str());
        delete[] shaderLog;
        return false;
    }

    *shaderId = shader;
    return true;
}

// Creates a new shader program, regardless of the number of shader stages. Stages are added in order of specification in the vector.
bool ShaderManager::CreateProgram(std::vector<GLuint> shaders, GLuint *program)
{
    // Create the program
    GLint compileStatus;
    *program = glCreateProgram();
    for (unsigned int i = 0; i < shaders.size(); i++)
    {
        glAttachShader(*program, shaders[i]);
    }

    glLinkProgram(*program);
    glGetProgramiv(*program, GL_LINK_STATUS, &compileStatus);
    if (!compileStatus)
    {
        GLint logLength;
        glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &logLength);

        char* buffer = new char[logLength];
        glGetProgramInfoLog(*program, logLength, &logLength, buffer);
        Logger::LogError("GLSL program compilation error: ", buffer);
        delete[] buffer;

        return false;
    }

    // These are auto-deleted when the program is deleted
    for (unsigned int i = 0; i < shaders.size(); i++)
    {
        glDeleteShader(shaders[i]);
    }

    return true;
}

// Creates a basic vertex-fragment shader program and adds it to the list of programs that will be deleted at the end of program operation
bool ShaderManager::CreateShaderProgram(const char *rootName, GLuint *programId)
{
    std::string vsShader, fsShader;
    if (!ReadShader(rootName, ".vs", &vsShader) || !ReadShader(rootName, ".fs", &fsShader))
    {
        return false;
    }
    
    GLuint vertexShader, fragmentShader;
    if (!CreateShader(GL_VERTEX_SHADER, vsShader.c_str(), &vertexShader) || !CreateShader(GL_FRAGMENT_SHADER, fsShader.c_str(), &fragmentShader))
    {
        return false;
    }

    std::vector<GLuint> shaders;
    shaders.push_back(vertexShader);
    shaders.push_back(fragmentShader);
    
    if (!CreateProgram(shaders, programId))
    {
        return false;
    }

    shaderPrograms.push_back(*programId);
    return true;
}

// Creates a vertex-geomtry-shader program.
bool ShaderManager::CreateShaderProgramWithGeometryShader(const char *rootName, GLuint *programId)
{
    std::string vsShader, gsShader, fsShader;
    if (!ReadShader(rootName, ".vs", &vsShader) || !ReadShader(rootName, ".gs", &gsShader) || !ReadShader(rootName, ".fs", &fsShader))
    {
        return false;
    }

    GLuint vertexShader, geometryShader, fragmentShader;
    if (!CreateShader(GL_VERTEX_SHADER, vsShader.c_str(), &vertexShader)
        || !CreateShader(GL_GEOMETRY_SHADER, gsShader.c_str(), &geometryShader)
        || !CreateShader(GL_FRAGMENT_SHADER, fsShader.c_str(), &fragmentShader))
    {
        return false;
    }

    std::vector<GLuint> shaders;
    shaders.push_back(vertexShader);
    shaders.push_back(geometryShader);
    shaders.push_back(fragmentShader);

    if (!CreateProgram(shaders, programId))
    {
        return false;
    }

    shaderPrograms.push_back(*programId);
    return true;
}

// Creates a shader program with vertex-tesselation_control-tesselation_evaluation-fragment stage sequence.
bool ShaderManager::CreateShaderProgramWithTesselation(const char *rootName, GLuint *programId)
{
    std::string vsShader, tcShader, teShader, fsShader;
    if (!ReadShader(rootName, ".vs", &vsShader) || !ReadShader(rootName, ".tc", &tcShader) 
        || !ReadShader(rootName, ".te", &teShader) || !ReadShader(rootName, ".fs", &fsShader))
    {
        return false;
    }

    GLuint vertexShader, tesselationControlShader, tesselationEvaluationShader, fragmentShader;
    if (!CreateShader(GL_VERTEX_SHADER, vsShader.c_str(), &vertexShader)
        || !CreateShader(GL_TESS_CONTROL_SHADER, tcShader.c_str(), &tesselationControlShader)
        || !CreateShader(GL_TESS_EVALUATION_SHADER, teShader.c_str(), &tesselationEvaluationShader)
        || !CreateShader(GL_FRAGMENT_SHADER, fsShader.c_str(), &fragmentShader))
    {
        return false;
    }

    std::vector<GLuint> shaders;
    shaders.push_back(vertexShader);
    shaders.push_back(tesselationControlShader);
    shaders.push_back(tesselationEvaluationShader);
    shaders.push_back(fragmentShader);

    if (!CreateProgram(shaders, programId))
    {
        return false;
    }

    shaderPrograms.push_back(*programId);
    return true;
}

// Deletes the specified shader program, removing it from the list of known programs.
void ShaderManager::DeleteShaderProgram(GLuint program)
{
    glDeleteProgram(program);
    for (unsigned int i = 0; i < shaderPrograms.size(); i++)
    {
        if (shaderPrograms[i] == program)
        {
            shaderPrograms.erase(shaderPrograms.begin() + i);
            break;
        }
    }
}

ShaderManager::~ShaderManager()
{
    for (unsigned int i = 0; i < shaderPrograms.size(); i++)
    {
        glDeleteProgram(shaderPrograms[i]);
    }

    shaderPrograms.clear();
}
