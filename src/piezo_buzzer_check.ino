 int piezo=8;//피에조 부저는 3번 핀으로 설정  
 int numTones=8;//numTones이라는 배열 수를 나타내는 변수 
 int tones[]={261,294,330,349,392,440,494,523}; 
//각각의 도레미파솔라시도 

void setup(){ 
 pinMode(piezo,OUTPUT);//피에저부조가 연결된 핀을 output으로 설정 
   for(int i=0; i<numTones; i++){ 
     tone(piezo,tones[i]); 

     delay(150); 
    } 
    noTone(piezo); 
  }  
 
void loop(){ 

} 
