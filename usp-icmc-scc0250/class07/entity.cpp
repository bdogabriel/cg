#include "entity.h"

void Entity::build()
{
    trs::build(matrix, transform);
}

void Entity::draw(GLint locTransform, GLint locColor) const
{
    glUniform4f(locColor, color.r, color.g, color.b, color.a);
    glUniformMatrix4fv(locTransform, 1, GL_FALSE, matrix.data());
    geometry.draw();
}
