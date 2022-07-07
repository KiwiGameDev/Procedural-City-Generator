
# Procedural Philippine City Generation using Real World Road Network Data

A tool to synthetically generate a Manila-based city. Made using Unreal Engine 4.




## Authors
[Bruce Anakin Miranda](https://github.com/KiwiGameDev), [Jacob Manzano](https://github.com/PigletGD), [Jan Carlo San Juan](https://github.com/jancarlosanjuan), [Jaso Jacob Santiago](https://github.com/JasoSantiago)
## Abstract
Manual content generation of complex systems is tedious and time consuming which makes many projects that rely on the production of complex systems infeasible to do and maintain. Procedural content generation (PCG) solves this problem by providing a limitless source of generated content, but will always be only good at generating the complex systems it was designed to generate. Because technology is starting to become more accessible and efficient, countries have the opportunity to utilize and adapt these new technologies for the betterment of society. For example, cities can use computer-vision to create surveillance systems to monitor the flow of traffic, making research for city planning much easier. With simulations of tasks such as computer-vision requiring a large amount of synthetic data, this makes procedural city generation important. There exists software that deals with city generation, with Parish and Müller’s CityEngine being the most influential of the ones publicly available, most software only handles generation of generic cities. There is a lack of study on city generation that is specialized to create cities based on a specific country. This paper proposes a system that is capable of  procedurally generating cities that attempts to closely mimic the aesthetics and qualities of Philippine cities.
## Usage
Follow the instructions below to compile and run the project from source.

### Windows
#### Installation
- Install Unreal Engine 4.26.2 & its dependencies
- Install Microsoft Visual Studio 2019
- Clone repo (with LFS)
- Download the [_CONTENTS_](https://drive.google.com/drive/folders/1esjHHMma-DroA2Bx1vJE48VVC7kaPr30?usp=sharing) folder and extract to the root folder
- Open the solution file and build

#### Minimum Requirements
- 8 Core CPU
- 8 GB RAM
- GeForce GTX 10 Series or AMD RX500 Series or above

Other platforms are currently not supported.
## Testing Procedures
Our training procedure is done in batches for each real-life reference image. The script that handles this is located in ```./scripts/testing/run.py```. To successfully run the test, you need to have a folder within the same directory named ```images```. Inside it are two (2) subfolders: ```true``` and ```unreal```. Inside ```true``` are your reference images. The images inside need to follow a naming convention and need to be a png. The name of the images need to be ```m_(n).png``` where n is sequentially numbered with how many you want to test. For example if you want to test three (3) reference images, there will be ```m_1.png```, ```m_2.png```, and ```m_3.png```. Inside ```unreal``` are the Seeds.txt file generated when generating from the application, and two (2) other folders: skewed and unskewed. Inside the ```skewed``` folder are the assisted-generation images and inside the ```unskewed``` folder are the non-assisted-generation images. Keep in mind that all of these images need to be the same dimension. Once everything is correctly placed, you can then simply execute ```run.py```.

```
python run.py
```
## Acknowledgements
We would like to acknowledge De La Salle University (DLSU), Sir Neil Patrick DelGallego, and Dr. Rafael Cabredo.