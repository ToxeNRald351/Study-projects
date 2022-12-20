import soundfile
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as signal

from matplotlib import gridspec

f = open("out.txt", "w")

data, fs = soundfile.read('xhavlo01.wav')
#Zaklady-------------------------------------------------------------------------------------------------------
f.write("4.1 Zaklady")
f.write("\n" + "In samples: " + str(data.size) + "\nIn seconds: " + str(data.size/fs))
f.write("\n" + "Minimum: " + str(data.min()*2**15) + "\nMaximum: " + str(data.max()*2**15))

duration = np.arange(data.size)/fs

fig = plt.figure(figsize = (12, 4))
gs = gridspec.GridSpec(6, 3)

plt.plot(duration, data)
plt.gca().set_xlabel('$t[s]$')
plt.gca().set_title('Zvukový signál')
plt.grid()
plt.tight_layout()
plt.savefig('Signal_look.png')
f.write("\n" + "See 'Signal_look.png' for result")
#Predspracovani a ramce-----------------------------------------------------------------------------------------
f.write("\n" + "\n4.2 Predspracovani a ramce")
#stredni hodnota
strhod = np.mean(data, axis=0)
#ustrednit data
data = [x-strhod for x in data]
f.write("\n" + "Normalized minimum: " + str(np.min(data)) + "\nNormalized maximum: " + str(np.max(data)))

rms = [block for block in
       soundfile.blocks('xhavlo01.wav', blocksize=1024, overlap=512)]
sum = 0
best_block = []
# Find pekny signal
for block in rms[:-1]:
    w, h = signal.freqz(block, fs=fs)
    min = 2**15
    for elem in h:
        sum += np.abs(elem)
    if (sum < min):
        best_block = block
        min = sum
time = np.arange(len(best_block))/fs

plt.gca().clear()
plt.plot(time, best_block)
plt.gca().set_xlabel('$t [s]$')
plt.gca().set_title('"Pěkný" signal')
plt.grid()
plt.savefig('Pekny_Signal.png')
f.write("\n" + "See 'Pekny_Signal.png' for result")
#DFT------------------------------------------------------------------------------------------------------------------------
f.write("\n" + "\n4.3 DFT")

def my_dft(a):
    n = len(a)
    if n == 1:
        return a
    omega = np.exp(2*np.pi*1j/n)
    Pe, Po = a[::2], a[1::2]
    ye, yo = my_dft(Pe), my_dft(Po)
    y = [0] * n
    for j in range(n//2):
        y[j] = ye[j] + omega**j * yo[j]
        y[j + n//2] = ye[j] - omega**j * yo[j]
    return y

f.write("\n" + "Use np.fft.fft function on 'Pěkný signal'. See 'Standard FFT func.png' for result")
dft_result = np.fft.fft(best_block)
n = 512
a = [((fs/2)/n)*x+1 for x in range(512)]

plt.gca().clear()
plt.plot(a, np.abs(dft_result[:512]))
plt.gca().set_xlabel('$Freq [Hz]$')
plt.gca().set_title('Numpy FFT function result')
plt.grid()
plt.savefig('Standard FFT func.png')

f.write("\n" + "Use my DFT function on 'Pěkný signal'. See 'My DFT func.png' for result")
dft_result = my_dft(best_block)

plt.gca().clear()
plt.plot(a, np.abs(dft_result[:512]))
plt.gca().set_xlabel('$Freq [Hz]$')
plt.gca().set_title('My DFT function result')
plt.grid()
plt.savefig('My DFT func.png')
#Spectrogram----------------------------------------------------------------------------------------------------------------------
f.write("\n" + "\n4.4 Spectrogram")

plt.gca().clear()
def make_spec(data, fs, name):
    fig, ax = plt.subplots(1)
    ax.specgram(x=data, Fs=fs, noverlap=512, NFFT=1024)
    fig.gca().set_xlabel('$Time [s]$')
    fig.gca().set_ylabel('$Freq [Hz]$')
    fig.savefig(name)
    f.write("\n" + "See" + name + " for result")
    return
make_spec(data, fs, 'Signal spec.png')
#Urceni rusivich freqvenci--------------------------------------------------------------------------------------------------------
f.write("\n" + "\n4.5 Určení rušivích freqvencí")
n = len(data)                                   #number of samples
dt = 1/fs
freq = (1/(dt*n)) * np.arange(n)
fhat = np.fft.fft(data, n)                      #mods
PSD = fhat * np.conj(fhat) / n                  #power spectral density
L = np.arange(1, np.floor(n/2), dtype='int')    #only use 1/2 of array

indices = PSD > 0.1
PSDnoise = PSD * indices
noise_freqs = [0, 0, 0, 0]

f.write("\n" + "Rušivé freqvence: ")
j = 0
i = 0
for x in PSDnoise[L]:
    if x != 0:
        noise_freqs[i] = freq[j]
        i += 1
        f.write("\n" + str(freq[j]) + " Hz")
    j += 1
#Generování signalu-------------------------------------------------------------------------------------------------------------------
f.write("\n" + "\n4.6 Generování signalu")
fhat_noisy = np.fft.ifft(fhat * indices, n)
soundfile.write('4cos.wav', np.float64(np.real(fhat_noisy)), fs)
f.write("\n" + "Check '4cos.wav' for result")
make_spec(np.float64(np.real(fhat_noisy)), fs, 'Noise spec.png')
#Čisticí filtr + Freq chars + Filtrace------------------------------------------------------------------------------------------------
f.write("\n" + "\n4.7 Čisticí filtr + 4.9 Freq Chars + 4.10 Filtrace")

def plot_char(a, b, fs, x):
    fig, ax = plt.subplots()
    ax.set_title('Digital filter #' + str(x) + ' frequency response')
    w, h = signal.freqz(b=b, a=a, fs=fs)
    ax.plot(w, 20 * np.log10(abs(h)), 'b')
    ax.set_ylabel('Amplitude [dB]', color='b')
    ax.set_xlabel('Frequency [Hz]')
    ax2 = ax.twinx()
    angles = np.unwrap(np.angle(h))
    ax2.plot(w, angles, 'g')
    ax2.set_ylabel('Angle (radians)', color='g')
    ax2.grid()
    ax2.axis('tight')
    fig.savefig('Filter #' + str(x) + ' chars')
    return

def plot_imp_response(a, b, n, x):
    system = (b, a)
    t, y = signal.impulse(system, N=n)
    
    fig, ax = plt.subplots()
    ax.set_title("Impulse response of filter #" + str(x))
    ax.set_ylabel('$Impulse$')
    ax.set_xlabel('$Time [s]$')
    ax.plot(t, y)
    fig.savefig("Filter response #" + str(x) + ".png")
    return

def clear_multi(sample, noise_freqs, fs, width=30.0):
    x = 1
    for freqcut in noise_freqs:
        lowpass = (freqcut - width)/(fs/2)
        highpass = (freqcut + width)/(fs/2)
        Wn = [lowpass, highpass]
        b, a = signal.butter(5, Wn, btype='bandstop')
        plot_char(a, b, fs, x)
        plot_imp_response(a, b, len(sample), x)
        sample = signal.filtfilt(b, a, sample)
        x += 1
    return sample

clear_sig = clear_multi(data, noise_freqs, fs)

soundfile.write('clean_bandstop.wav', clear_sig, fs)
f.write("\n" + "Check 'clean_bandstop.wav' for result")
f.write("\n" + "See different 'Filter #X chars.png' for result")
#Nulové body a poly-------------------------------------------------------------------------------------------------------------
f.write("\n" + "\n4.8 Nulové body a poly")

def zeros_poles(noise_freqs, fs, width=30.0):
    fig, ax = plt.subplots(2)
    iswhat = ['Zeros', 'Poles']
    for x in range(2):
        ax[x].grid(True)
        ax[x].set_xlabel('$Real$')
        ax[x].set_ylabel('$Imaginary$')
        ax[x].set_title(iswhat[x])
    for freqcut in noise_freqs:
        lowpass = (freqcut - width)/(fs/2)
        highpass = (freqcut + width)/(fs/2)
        Wn = [lowpass, highpass]
        z, p, k = signal.butter(5, Wn, btype='bandstop', output='zpk')
        x = [ele.real for ele in p]
        y = [ele.imag for ele in p]
        ax[0].scatter(x, y, c='red')
        x = [ele.real for ele in z]
        y = [ele.imag for ele in z]
        ax[1].scatter(x, y, c='blue')
        
    fig.savefig('Zeros and poles.png')
    return

zeros_poles(noise_freqs, fs)
f.write("\n" + "See 'Zeros and poles.png' for result\n")
f.close()
print("Done")
