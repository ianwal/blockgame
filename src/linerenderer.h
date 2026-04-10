#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <map>
#include <vector>

namespace bg
{

struct Line {
    glm::vec3 start{};
    glm::vec3 end{};
    float width{ 5.0f };
    glm::vec4 color{ 1.0f };
};

class LineRenderer
{
public:
    // Must be called before render.
    static void init();

    static void cleanup();

    static void render(Line const& line);

private:
    static GLuint vao;
    static GLuint vertexbuffer;
    static GLuint shaderId;
    static GLuint colorId;
};

} // namespace bg
