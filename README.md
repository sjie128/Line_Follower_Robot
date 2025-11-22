# REC Team Recruitment 2
## Group 2 Members

| Mechanical | Electronic | Programming |
|----------|----------|-------|
| Chee Wei Ian | Goh Wei Qin | Chang Yong Qi |
| free rider | Hong Rui Leong | Janisa Goh Shi Jie |

# 🚗 Line Following Robot with QTR Sensors, Servo Gate & Checkpoints  
Arduino-based PID Line Follower with object-drop mechanism

## 📌 Overview  
This repository contains the Arduino code for a **5-IR-sensor line following robot** using a **QTR reflectance array**, **servo gate mechanism**, and **checkpoint-based automation**.

The robot is capable of:

- Smooth line following  
- Detecting checkpoints  
- Opening/closing a gate to drop objects  
- Non-blocking movements using `millis()`  
- 90° sharp turns (left & right)  
- Lost-line handling and recovery  

---

## 🧰 Hardware Used  

| Component | Quantity | Notes |
|----------|----------|-------|
| Arduino Uno / Nano | 1 | Main controller |
| QTR-5RC or QTR-5A Sensor Array | 1 | Line sensing |
| L298N Motor Driver | 1 | Controls left/right motors |
| DC Motors | 2 | Differential drive |
| Servo Motor | 1 | Gate mechanism |
| Tactile Push Button | 1 | Start button |
| 5 IR Digital Sensors | 5 | Additional digital sensing |
| Power Supply | 1 | 6–12V |

---

## ⚙️ Features  

### ✔️ Automatic Calibration  
Uses Pololu **QTRSensors** library to calibrate reflectance.

### ✔️ Line Following (PD-style Correction)  
Weights: `{-4, -2, 0, 2, 4}`  
Adjusted using a configurable gain.

### ✔️ Checkpoint System  
Detects black patches and performs actions:

| Checkpoint | Action |
|-----------|--------|
| 1 | Move forward extra distance |
| 2 | Drop object |
| 3 | Drop object |
| 4 | Drop object |
| 5 | Move forward & fully stop |

### ✔️ Servo Gate Mechanism  
- Closed: **120°**  
- Open: **180°**

### ✔️ Sharp Turn Detection  
Triggers when special IR patterns detected and robot spins until aligned.

### ✔️ Non-Blocking Timing  
All forward, dropping, spinning actions use `millis()` instead of `delay()`.

---

## 📂 File Structure  
├── src/
│ └── main.ino # Main Arduino code
├── README.md # Documentation (this file)
└── wiring/ # Wiring diagrams (optional)


---

## 🔌 Wiring Summary  

### **Motor Driver (L298N)**

| L298N Pin | Arduino |
|----------|---------|
| ENA | 3 |
| IN1 | 2 |
| IN2 | 4 |
| IN3 | 5 |
| IN4 | 7 |
| ENB | 6 |

### **QTR Sensor Array (5 sensors)**  
Pins: `18, 17, 16, 15, 14`  *(A4 → A0)*

### **Button**  
Pin: `10`

### **Servo Gate**  
Pin: `9`

---

## ▶️ How to Use  

### 1️⃣ Power on robot  
QTR array auto-calibrates for ~2 seconds.

### 2️⃣ Press the button  
Exits calibration mode and starts line following.

### 3️⃣ Robot performs actions at each checkpoint  
Gate opens, drops cube, then closes.

### 4️⃣ Robot stops after checkpoint 5  

---

## 🧪 Troubleshooting  

### ⚠️ Servo not moving  
- Check 5V supply  
- Ensure common ground  
- Gate angle values correct  

### ⚠️ Robot shaky  
Tune values in code:



