blockSize = 65536;

f = fopen('entropy-sda1.txt', 'r');
ha = fscanf(f, '%f');
fclose(f);

f = fopen('entropy-sdb1.txt', 'r');
hb = fscanf(f, '%f');
fclose(f);

len = min(length(ha), length(hb));
ha = ha(1:len);
hb = hb(1:len);

ha = resample(ha,1,80);
hb = resample(hb,1,80);

gb = linspace(0, len*blockSize/(1024*1024*1024)-1, length(ha));
plot(gb,ha,'r*',gb,hb,'b.');
legend('sda1 (botched)', 'sdb1 (reference)');
xlabel('Disk offset (GB)');
ylabel('Entropy');
