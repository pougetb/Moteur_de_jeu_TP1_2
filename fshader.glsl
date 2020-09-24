#version 150

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_snowrocks;

in vec2 v_texcoord;
in vec4 v_position;
//! [0]
void main()
{
    // Set fragment color from texture
    if(v_position.z > 0.66){
        gl_FragColor = texture2D(texture_snowrocks, v_texcoord);
    }else if(v_position.z > 0.33){
        gl_FragColor = texture2D(texture_rock, v_texcoord);
    }else{
        gl_FragColor = texture2D(texture_grass, v_texcoord);
    }

}
//! [0]

