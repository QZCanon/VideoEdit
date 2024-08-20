#version 440     
 in vec4 Color;   //汉字用于测试汉字是否可用，有报着色器源码注释含汉字运行报错的
 out vec4 FragColor;    
 void main()  
 {  
     FragColor = Color;  
 }  