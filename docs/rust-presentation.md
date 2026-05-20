# Presentation Framework: The Case for Rust in Embedded & Robotics

## Section 1: The "Why are we here?" (The Problem)
**Goal:** Establish that while C++ is the legend, it’s becoming a liability in modern, complex robotics.

### Slide 1: The Status Quo vs. The Complexity Wall
* **The Hook:** We are building robots that are more autonomous, more connected, and more concurrent than ever.
* **The Conflict:** C++ was designed for a different era. Today, 70% of our time is spent debugging memory, not building features.
* **Key Fact:** Industry giants like Microsoft and Google report that **70% of all high-severity security vulnerabilities** are memory safety issues (buffer overflows, use-after-free) [1].

### Slide 2: The "Human Discipline" Trap
* **Concept:** C++ safety depends on the "perfect programmer." In a team of students or engineers, someone *will* forget a pointer check.
* **The Robotics Risk:** In embedded, a memory bug isn't just a "Blue Screen"—it’s a drone falling out of the sky or a robotic arm crashing into hardware.

---

## Section 2: Rust’s "Secret Sauce" (The Technical Edge)
**Goal:** Explain *how* Rust solves these problems without losing the performance of C++.

### Slide 3: The Borrow Checker (Memory Safety without GC)
* **Fact:** Unlike Java or Python, Rust has **No Garbage Collector**.
* **The Pitch:** It uses an "Ownership" system. The compiler checks memory at *compile time*. If your code compiles, it is mathematically guaranteed to be memory-safe.
* **Analogy:** C++ is like flying a plane where you have to manually check every bolt mid-flight. Rust is like a pre-flight inspection that won't let the engine start if a bolt is loose.

### Slide 4: Fearless Concurrency
* **Fact:** Robotics involves many things happening at once (Sensors, PID loops, Motors).
* **The Pitch:** Rust prevents "Data Races" at compile-time. You cannot have two threads fighting over the same motor controller variable. In C++, this is a nightmare to debug; in Rust, it’s a compiler error.

### Slide 5: Zero-Cost Abstractions & Performance
* **Hard Fact:** Benchmarks show Rust is typically within **5-10% of C++ performance** in microbenchmarks, but often **beats C++ in real-world scenarios** because the compiler can optimize safer code more aggressively [2].
* **Power Efficiency:** Frameworks like **Embassy** allow for "Async" code that puts the CPU to sleep automatically when waiting for sensors, significantly increasing battery life [3].

---

## Section 3: The Developer Experience (The "Quality of Life")
**Goal:** Show your classmates that their lives will be easier.

### Slide 6: Cargo vs. The "CMake Pain"
* **The Pain:** Everyone in class has spent 3 hours trying to get a C++ library to link in CMake.
* **The Solution:** **Cargo**. One command (`cargo build`) handles your dependencies, compilation, and testing.
* **Fact:** Rust’s adoption has grown by **nearly 70% since 2021**, with over 2.2 million developers, largely due to this superior tooling [2].

### Slide 7: Typestate & Hardware Safety
* **The Concept:** Using Rust’s type system to represent hardware states.
* **Example:** You can write code where it is *physically impossible* to send data to a pin unless that pin has been configured as an Output. The code won't compile otherwise.

---

## Section 4: The Robotics Ecosystem
**Goal:** Prove that Rust is ready for professional robotics.

### Slide 8: RTIC & Embassy (The Modern RTOS)
* **RTIC (Real-Time Interrupt-driven Concurrency):** Best for hard real-time tasks.
* **Embassy:** Modern "Async/Await" for embedded. It eliminates the "RTOS Tax" (no manual stack size configuration) and turns tasks into state machines at compile-time [3].

### Slide 9: ROS2 & The Future
* **Integration:** Mention `ros2rust` and `roslibrust`.
* **The Shift:** Android reduced memory safety vulnerabilities from **76% to 24%** over 6 years by shifting to memory-safe languages like Rust [1]. Robotics is the next logical frontier.

---

## Section 5: The "Start Today" Plan
**Goal:** Give them an actionable takeaway.

### Slide 10: How to Transition
1.  **Don't Delete, Integrate:** Use FFI (Foreign Function Interface) to keep your old C++ drivers while writing your high-level logic in Rust.
2.  **Pick a Project:** Try your next ESP32 or STM32 project using the **Embassy framework**.
3.  **The Challenge:** "If you start your next project in C++, you are choosing to spend 30% of your time on bugs that shouldn't exist."

---

## References & Data Points for your Notes:
1. **Google/Microsoft Stat:** 70% of CVEs are memory safety issues. Android's switch to Rust saw a 50%+ drop in these vulnerabilities.
2. **Performance:** The "Computer Languages Benchmark Game" shows Rust is 95-100% the speed of C++ while being memory safe.
3. **Embassy Benefits:** Eliminates "stack overflow" crashes by calculating memory needs at compile-time.
