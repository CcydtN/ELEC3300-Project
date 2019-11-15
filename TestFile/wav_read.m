[x,fs] = audioread("A4_Tone.wav",'native');
x = x';
t = 0:1/fs:2/440-1/fs;
x = typecast(x,'uint16');
x = bitxor(x,uint16(ones(1,length(x))*2^(15)));
l = length(t);

plot(t,x(1,1:l));