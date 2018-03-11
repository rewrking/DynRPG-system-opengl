uniform sampler2D texture;

const mat4 mDeuteranopia = mat4( 0.43 ,  0.72 , -0.15 ,  0.0 ,
                                 0.34 ,  0.57 ,  0.09 ,  0.0 ,
                                -0.02 ,  0.03 ,  1.00 ,  0.0 ,
                                 0.0  ,  0.0  ,  0.0  ,  1.0 );

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	gl_FragColor = pixel*mDeuteranopia;
}
