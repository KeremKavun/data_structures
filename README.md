# 🛠️ Data Structures in C

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Language](https://img.shields.io/badge/language-C-orange.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)

A collection of high-performance, modular data structure implementations developed during a formal Data Structures course, featuring professional C idioms and pointer safety.

---

## 📖 Overview

This repository provides a robust library of fundamental data structures. Each module is self-contained, including dedicated headers, source files, and unit tests to ensure reliability.

### 📂 Project Structure
| Directory | Purpose |
| :--- | :--- |
| `include/` | Public API headers and definitions. |
| `src/` | Core implementation logic and algorithms. |
| `tests/` | Comprehensive unit and integration tests. |

---

## 🚀 Modules & References

| Module | Description | Links |
| :--- | :--- | :--- |
| **Arrays** | Contiguous memory wrappers & Dynamic Vectors. | [Headers](./include/ds/arrays/) | [Source Code](./src/arrays/) |
| **Linked Lists** | Intrusive circular and linear structures. | [Headers](./include/ds/linkedlists/) | [Source Code](./src/linkedlists/) |
| **Trees** | Binary, Heaps, AVL, and BST. | [Headers](./include/ds/trees/) | [Source Code](./src/trees/) |
| **Queue/Stack** | FIFO and LIFO (with stack-frame safety). | [Headers](./include/ds/queue/) / [Source Code](./src/queue/) | [Headers](./include/ds/stack/) / [Source Code](./src/stack/) |
| **Graphs** | Adjacency-list for directed/unweighted graphs. | [Headers](./include/ds/graphs/) | [Source Code](./src/graphs/) |
| **Hash Structs** | High-performance lookup tables. | [Headers](./include/ds/hashs/) | [Source Code](./src/hashs/) |
| **Utilities** | Logging, Object concepts, and Allocators. | [Headers](./include/ds/utils/) | [Source Code](./src/utils/) |

---

## 🧠 Development Philosophy

This project leverages a hybrid human-AI development workflow to achieve professional architectural standards:

* **Architecture & Design:** Collaborated with **Gemini** to implement industry-standard C idioms and modularity.
* **Quality Assurance:** **Claude**-generated unit tests provide edge-case verification.
* **Documentation:** Standardized via **Doxygen** for machine-readable and human-friendly guides.

---

## 🛠️ Building & Usage

### Compile the Library
```sh
make
```

### Run Tests

#### Run everything
```sh
make test_all
```

#### Run specific module
```sh
make test_<module_name>
```

### Documentation
```sh
make docs
```

## 📧 Contact
If you have questions or want to discuss the implementations, feel free to reach out:
* **Email:** [209keremkavun@gmail.com](mailto:209keremkavun@gmail.com)

## 📜 License
Distributed under the MIT-like permissive license. See LICENCE.md for more information.