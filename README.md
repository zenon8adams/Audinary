<p align="center">
  <img align="center" src="https://github.com/zenon8adams/audinary/blob/master/icon.png" alt="screenshot"/>
</p>

<p align="center">
  <a title="Distro" target="_blank" href="https://linuxmint.com/"><img src="https://img.shields.io/badge/Linux_Mint-87CF3E?style=for-the-badge&logo=linux-mint&logoColor=white"></a>
  <a title="Crowdin" target="_blank" href="https://crowdin.com/project/audinary"><img src="https://badges.crowdin.net/audinary/localized.svg"></a>
</p>

## About Audinary

Audinary is a cinnamon desklet that displays time in text format. It provides:

<ul>
<li>Font customization</li>
<li>Background and color customization</li>
<li>Custom hour notification</li>
<li>Time presentation in all languages</li>
</ul>

## Screenshots

<p float="left">
  <img src="https://github.com/zenon8adams/audinary/blob/master/audinary%40zener-diode/screenshot.png"width="200" />
  &nbsp; &nbsp; &nbsp; &nbsp;
  <img src="https://github.com/zenon8adams/audinary/blob/master/screenshot_bg.png" width="200"/> 
  &nbsp; &nbsp; &nbsp; &nbsp;
  <img src="https://github.com/zenon8adams/audinary/blob/master/screenshot_catalan.png" width="200" />
</p>

## Dependencies

Audinary depends on:

[cAudio](https://github.com/R4stl1n/cAudio): For notification

[libhttpserver](https://github.com/etr/libhttpserver): For communication with the time daemon

[audinary-pack](https://github.com/zenon8adams/audinary-pack): For translation to your locale

## Building
```
mkdir build
cd build
../bootstrap.sh
../configure --sysconfdir=/etc
make
```

## Installing
From within build directory, run:
```
sudo make install
```







