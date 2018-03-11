uniform sampler2D texture;

void main() {
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

	gl_FragColor = vec4(1 - pixel.r, 1 - pixel.g, 1 - pixel.b, pixel.a) * gl_Color;
}