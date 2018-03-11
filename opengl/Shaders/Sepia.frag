uniform sampler2D texture;

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	//float gray = (pixel.r + pixel.g + pixel.b) / 3;
	float gray = dot(pixel.rgb, vec3(0.299, 0.587, 0.114));
	
	float red=gray;
	float green=gray*.75;
	float blue=gray*.5;
	
	gl_FragColor = vec4(red, green, blue, pixel.a) * gl_Color;
}
