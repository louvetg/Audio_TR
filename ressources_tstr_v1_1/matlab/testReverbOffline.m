% Convolutional reverb - Offline
% T. Hueber - CNRS/GIPSA-lab
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all;
close all;

% read the sample waveform
filename='./MG_list1_sent381.wav';
[x,Fs] = audioread(filename);

i = 0;
x = x(1:258*512);
X = reshape(x, [], 258);

% read the impulse response waveform
filename='./IMreverbs1/Five Columns.wav';
[imp,Fsimp] = audioread(filename);

% Keep only one channel 
imp_left = imp(:,1);

M = length(imp_left);
L = length(x);
L_OA = 512;

% %% Overlap Add
x_conv = zeros(1,L_OA+M-1);
out = zeros(512,258);
bufferTmp = zeros(1,M-1);
% 
% 
% for i = 1 : 258
%     bufferIn =  X(:,i);
%   
%     for n=1 : L_OA + M-1    
%         tmp = 0;
%         if (n>=M)
%             kmin = n-M+1;
%         else
%             kmin = 1;
%         end
%     
%         if (n<L_OA)
%             kmax = n;
%         else
%             kmax = L_OA;
%         end
%         
%         for k=kmin:kmax
%             tmp = tmp + bufferIn(k)*imp_left(n-k+1);
%         end
%         
%         if(n < M)
%             if n< L_OA+1
%             bufferOut(n)= tmp + bufferTmp(n);
%             else
%             bufferTmp(n - L_OA) = tmp + bufferTmp(n);
%             end
%         else 
%             bufferTmp(n - L_OA) = tmp;
%         end
%         
%         Ly = 512 + length + L_OA;
%     end
%     
%     out(:,i)=bufferOut;
% end
% 
% tst = reshape(out,[],1);
% 
% %% Convolution normal
% 
% x_conv = zeros(1,L+M-1);
% 
%   for n=1:L+M-1
%     tmp = 0;
%     if (n>=M)
%       kmin = n-M+1;
%     else
%       kmin = 1;
%     end
%     
%     if (n<L)
%       kmax = n;
%     else
%       kmax = L;
%     end
%     %fprintf('kmin=%i,kmax=%i,n=%i\n',kmin,kmax,n);
%     for k=kmin:kmax
%       tmp = tmp + x(k)*imp_left(n-k+1);
%     end
%     x_conv(n)=tmp;
%   end
% 
% tst2 = x_conv;

%% fr�quentiel

x_conv = zeros(1,L_OA+M-1);
out = zeros(512,258);
FFT_tempon = zeros(1,M-1);

FFT_length = 512 + M;
FFT_length_pow2=pow2(nextpow2(FFT_length)); 

FFT_data_RI_x= fft(imp_left, FFT_length_pow2);		   % Fast Fourier transform



for i = 1 : 258
    bufferIn =  X(:,i);
    
    FFT_data = fft(bufferIn,FFT_length_pow2);
    
    FFT_result = FFT_data .* FFT_data_RI_x;
    
    FFT_result = real(ifft(FFT_result, FFT_length_pow2));      
    FFT_result = FFT_result(1:1:FFT_length);               
    
    for n = 1 : L_OA + M-1 
        if(n < M)
            if n< L_OA+1
            bufferOut(n)= FFT_result(n) + bufferTmp(n);
            else
            bufferTmp(n - L_OA) = FFT_result(n) + bufferTmp(n);
            end
        else 
            bufferTmp(n - L_OA) = FFT_result(n);
        end
    end
    out(:,i)=bufferOut;
end

tst = reshape(out,[],1);


%% END