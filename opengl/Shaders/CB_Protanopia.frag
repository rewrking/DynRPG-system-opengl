uniform sampler2D texture;

const mat4 mProtanopia = mat4( 0.20 ,  0.99 , -0.19 ,  0.0 ,
							   0.16 ,  0.79 ,  0.04 ,  0.0 ,
							   0.01 , -0.01 ,  1.00 ,  0.0 ,
							   0.0  ,  0.0  ,  0.0  ,  1.0 );

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	gl_FragColor = pixel*mProtanopia;
}
