uniform sampler2D texture;

//#define gamma 1.05

void main() {
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	//float gray = (pixel.r + pixel.g + pixel.b) / 3;
	float gray = dot(pixel.rgb, vec3(0.299, 0.587, 0.114));
	//gray = gray*gamma;

	gl_FragColor = vec4(gray, gray, gray, pixel.a) * gl_Color;
}