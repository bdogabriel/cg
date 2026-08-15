attribute vec2 position;
uniform mat4 mat_transformation;
void main() {
    gl_Position = mat_transformation * vec4(position, 0.0, 1.0);
}
