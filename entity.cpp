#include "entity.h"

void Entity::build_matrix()
{
    matrix = Mat4::identity();
    trs::translate(matrix, transform);

    trs::rotate(rotation, transform);
    matrix *= rotation;
    transform.rx = 0;
    transform.ry = 0;
    transform.rz = 0;

    trs::scale(matrix, transform);
}

void Entity::draw(GLint locTransform, GLint locColor) const
{
    glUniform4f(locColor, color.r, color.g, color.b, color.a);
    glUniformMatrix4fv(locTransform, 1, GL_FALSE, matrix.data());
    geometry.draw(primitive);
}
