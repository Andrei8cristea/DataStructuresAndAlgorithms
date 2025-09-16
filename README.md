# Sorting Algorithm Visualizer

This is a project I made to visualize how different sorting algorithms work. I used C++ and SFML for graphics, and it shows animations of arrays getting sorted.

---

## What's Inside

The program lets you visualize five different sorting algorithms:

- Insertion Sort  
- Selection Sort  
- Quick Sort  
- Merge Sort  
- Heap Sort

Each algorithm shows step-by-step how elements are compared, swapped, and moved into place with colorful animations.

---

## How to Use

- **Welcome Screen:** Click the **START** button or press **Enter** to begin  
- **Algorithm Selection:** Choose which sorting algorithm you want to see  
- **Visualization:** Watch the sorting happen with colorful animations!

**Controls:**

- Use mouse or arrow keys to navigate menus  
- Press **B** to go back to the previous screen  
- Press **Left / Right** arrows to adjust animation speed  
- Press **Space** or **P** to pause / resume the animation

---

## Technical Stuff

The code is organized into several parts:

- `CSorter` and derived classes handle the sorting algorithms  
- `CSortingVisualizer` manages the bar visualization  
- `CApp` handles the application flow and UI

Each algorithm records its steps into a buffer that's then animated.

The animations include:

- Color changes for comparisons  
- Smooth sliding for swaps  
- Special effects when elements reach their final positions

---

## About

I made this project to better understand how sorting algorithms work and to practice my C++ and SFML skills. 
