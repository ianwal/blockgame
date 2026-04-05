#pragma once

#include <GL/glew.h>

#include <cassert>
#include <map>
#include <string>
#include <string_view>

namespace bg
{

GLuint loadShader(std::string_view vertexPath, std::string_view fragmentPath);

GLuint loadTexture(std::string_view imagePath);

class ShaderManager
{
public:
    ShaderManager() = default;

    /// Get the shader by name.
    GLuint get(std::string const& name)
    {
        return programs.at(name);
    }

    /// Load the shader and assign it a name.
    ShaderManager& load(std::string const& name, std::string const& vert, std::string const& frag)
    {
        assert(!programs.contains(name));
        if (!programs.contains(name)) {
            programs[name] = loadShader(std::string_view{ vert }, std::string_view{ frag });
        }
        return *this;
    }

    void cleanup()
    {
        for (auto& p : programs) {
            glDeleteProgram(p.second);
        }
        programs.clear();
    }

    // Moveable, not copyable.
    ShaderManager(ShaderManager const&)            = delete;
    ShaderManager(ShaderManager&&)                 = default;
    ShaderManager& operator=(ShaderManager const&) = delete;
    ShaderManager& operator=(ShaderManager&&)      = default;

    ~ShaderManager()
    {
        cleanup();
    }

private:
    std::map<std::string, GLuint, std::less<>> programs{};
};

class ShaderManagerSingleton
{
public:
    static ShaderManager& get()
    {
        static ShaderManager singleton{};
        return singleton;
    }
};

class TextureResourceManager
{
public:
    TextureResourceManager() = default;

    /// Get the texture by name.
    GLuint get(std::string const& name)
    {
        return textures.at(name);
    }

    /// Load the texture and assign it a name.
    TextureResourceManager& load(std::string const& name, std::string const& path)
    {
        assert(!textures.contains(name));
        if (!textures.contains(name)) {
            textures[name] = loadTexture(std::string_view{ path });
        }
        return *this;
    }

    void cleanup()
    {
        for (auto& t : textures) {
            glDeleteProgram(t.second);
        }
        textures.clear();
    }

    // Moveable, not copyable.
    TextureResourceManager(TextureResourceManager const&)            = delete;
    TextureResourceManager(TextureResourceManager&&)                 = default;
    TextureResourceManager& operator=(TextureResourceManager const&) = delete;
    TextureResourceManager& operator=(TextureResourceManager&&)      = default;

    ~TextureResourceManager()
    {
        cleanup();
    }

private:
    std::map<std::string, GLuint> textures{};
};

class TextureResourceManagerSingleton
{
public:
    static TextureResourceManager& get()
    {
        static TextureResourceManager singleton{};
        return singleton;
    }
};

} // namespace bg
