uniform sampler2D texture;

#define color_depth 224.0
#define gamma 60.0
#define brightness 1.3

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	//float gray = (pixel.r + pixel.g + pixel.b) / 3;
	
	//vec4 f4 = pixel * vec4(0.299f, 0.587f, 0.114f, 1.0f);
	vec4 f = (floor(((pixel)/gamma)*color_depth)*(gamma*brightness))/color_depth;
	
	float red=f.r;
	float green=f.g;
	float blue=f.b;
	
	gl_FragColor = vec4(red, green, blue, pixel.a) * gl_Color;
}
