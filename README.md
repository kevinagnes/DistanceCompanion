# Distance Companion
![](https://github.com/kevinagnes/DistanceCompanion/blob/main/Pictures/distancecompanion_pixelart.gif)

## Table of Contents
1. [Introduction](#introduction)
2. [Components](#components)
3. [How to Use](#how-to-use)
4. [Development Notes](#development-notes)
5. [Conclusion](#conclusion)
6. [References and Resources](#references-and-resources)
7. [Libraries Used](#libraries-used)
8. [Contributing](#contributing)
9. [License](#license)
10. [Contact](#contact)


## Introduction
Distance Companion is a personalized communication platform that allows users to express their feelings and emotions to loved ones from a distance. It was inspired by the current world situation that has made it difficult for us to meet together physically. This project aims to bridge this gap in a playful and personal way, without trying to substitute traditional communication tools.

![System Overview](https://github.com/kevinagnes/DistanceCompanion/blob/main/Pictures/Process/SystemOverview.jpg)

## Components
The hardware for this project includes:
1. Microcontroller board: Adafruit Feather HUZZAH ESP8266 (integrated LiPo charger and booster)
2. Display: PCD8544 LCD Module (aka Nokia 5110)
3. LiPo Battery: 3.7V 1200mAh Rechargeable battery (PL603450)
![System Overview](https://github.com/kevinagnes/DistanceCompanion/blob/main/Pictures/Components/Components.jpg)

## How to Use
The Sender interface allows a user to create and send their current mood by manipulating a cartoonish face's features, creating a small drawing, or writing an eighty-character message. 

[You can still try the interface here](https://www.kevinagnes.com/distance-companion)

The Receiver is designed to connect to the internet every hour to receive messages. This saves battery time and makes the device not the center of attention for the user, but rather a surprise element that will positively impact the user during the day. Moreover, previous messages are retained on the display even when the Receiver is idling or sleeping.

## Development Notes

The project has gone through several development stages and iterations to establish the best communication and data storage methods.

Three sender applications were developed during the initial stages:
1. A custom HTML page generated from the Receiver. This approach was not suitable as it required the Sender and Receiver to share the same network.
2. An application using Open Sound Control (OSC) for real-time data transmission. This method proved to be more complex, requiring an additional smartphone app.
3. A web app that uses Google Sheets to store and transmit data from the Sender to the Receiver. This method was adopted for the project due to its simplicity and efficiency.

![](https://github.com/kevinagnes/DistanceCompanion/blob/main/Pictures/Process/InterfaceEvolution.jpg)

The Receiver interprets the incoming data and decides which program should be executed based on the first character of the sent data. It supports three types of messages:
1. Face: Modifies the face features (Eyes and Mouth) based on parameters.
2. Draw: Draws lines based on a sequence of (X,Y) vectors sent from the sender.
3. Text: Displays a message on the small screen.

![](https://github.com/kevinagnes/DistanceCompanion/blob/main/Pictures/Process/P5js_painting.jpg)

For the enclosure of the Receiver device, a shape resembling an old iMac was chosen, fulfilling the initial idea of a retro-tech design.

![](https://github.com/kevinagnes/DistanceCompanion/blob/main/Pictures/Enclosure/Model.png)


## Conclusion
The Distance Companion project successfully provides an engaging platform for communication with loved ones in a new and fun way. This project serves as a great learning outcome for studying network communication protocols (HTTP, TCP, UDP) and for efficient coding practices for power-saving devices.

## References and Resources
Here are some of the learning resources used during the development of this project:
- [ESP8266](https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html)
- [Saving Data in Google Sheets](https://www.benlcollins.com/spreadsheets/saving-data-in-google-sheets/)
- [Easier ESP8266 Development](https://ludzinc.blogspot.com/2019/07/easier-esp8266-development.html)
- [1 Year Sensor](https://blog.sarine.nl/2020/01/01/1-year-sensor.html)

## Libraries Used
Here are the libraries used in the project:
- [U8g2](https://github.com/olikraus/u8g2)
- [HTTPSRedirect](https://github.com/electronicsguy/ESP8266/tree/main/HTTPSRedirect)
- [OSC](https://github.com/CNMAT/OSC)

## Contributing
Contributions are what make the open-source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License
Distributed under the MIT License. See `LICENSE` for more information.

## Contact
Kevin Agnes - kevinagnes@gmail.com


