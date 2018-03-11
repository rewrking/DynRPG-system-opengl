uniform sampler2D texture;

const mat4 mAchromatopsia = mat4(0.299, 0.587 ,  0.114,  0.0 ,
								 0.299, 0.587 ,  0.114,  0.0 ,
								 0.299, 0.587 ,  0.114,  0.0 ,
                                 0.0  ,  0.0  ,  0.0  ,  1.0 );

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	gl_FragColor = pixel*mAchromatopsia;
}
