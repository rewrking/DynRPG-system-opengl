uniform sampler2D texture;

#define color_depth 255.0
#define gamma 75.0
#define brightness 1.2

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	vec4 out_pixel = pixel * vec4(0.15f, 0.3f, 0.4f, 1.0f);
	float f = (round(((out_pixel.r + out_pixel.g + out_pixel.b)/gamma)*color_depth)*(gamma*brightness))/color_depth;

	if (f < 0.25) {
		out_pixel = vec4(0.059, 0.22, 0.055, pixel.a);
	} else if (f < 0.50) {
		out_pixel = vec4(0.208, 0.384, 0.216, pixel.a);
	} else if (f < 0.75) {
		out_pixel = vec4(0.451, 0.627, 0.404, pixel.a);
	} else {
		out_pixel = vec4(0.608, 0.733, 0.055, pixel.a);
	}
	
	gl_FragColor = out_pixel * gl_Color;
}
