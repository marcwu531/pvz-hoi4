uniform sampler2D texture;
uniform float brightness;

void main() {
    vec4 color = texture2D(texture, gl_TexCoord[0].xy);
    color.rgb *= brightness;
    gl_FragColor = color;
}