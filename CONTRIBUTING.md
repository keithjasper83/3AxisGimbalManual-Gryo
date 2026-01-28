# Contributing to 3-Axis Gimbal Control System

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in Issues
2. If not, create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Hardware/software versions
   - Serial monitor output (if applicable)

### Suggesting Features

1. Check if the feature has been suggested
2. Create a new issue with:
   - Clear description of the feature
   - Use case and benefits
   - Possible implementation approach

### Code Contributions

#### Setup Development Environment

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/YOUR_USERNAME/3AxisGimbalManual-Gryo.git
   cd 3AxisGimbalManual-Gryo
   ```

3. Create a branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```

#### Coding Guidelines

**ESP32 Firmware (C++):**
- Follow Arduino/C++ naming conventions
- Use meaningful variable names
- Comment complex logic
- Keep functions focused and small
- Test on actual hardware

**Backend (Python):**
- Follow PEP 8 style guide
- Use type hints
- Write docstrings for functions
- Use async/await appropriately

**Documentation:**
- Clear and concise
- Include code examples
- Update README if needed
- Add images for hardware changes

#### Making Changes

1. Make your changes
2. Test thoroughly:
   ```bash
   # ESP32
   cd esp32_firmware
   pio run --target upload
   pio device monitor
   
   # Backend
   cd backend
   python main.py
   ```

3. Commit with clear messages:
   ```bash
   git add .
   git commit -m "Add: Brief description of changes"
   ```

4. Push to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```

5. Create a Pull Request

#### Pull Request Process

1. Update documentation
2. Ensure code passes any existing tests
3. Describe changes in PR description
4. Link related issues
5. Wait for review

### Documentation Contributions

Documentation improvements are always welcome:
- Fix typos
- Clarify instructions
- Add examples
- Improve diagrams

## Code of Conduct

### Our Standards

- Be respectful and inclusive
- Welcome newcomers
- Accept constructive criticism
- Focus on what's best for the community

### Unacceptable Behavior

- Harassment or discrimination
- Trolling or insulting comments
- Personal attacks
- Publishing others' private information

## Development Setup

### Required Tools

- PlatformIO Core or IDE
- Python 3.8+
- Git
- Text editor or IDE

### Hardware for Testing

- ESP32 development board
- MPU6050 sensor
- 3x servo motors
- Power supply

### Testing

Before submitting:
- Test on actual hardware
- Verify web interface works
- Check serial monitor for errors
- Test both WiFi and hotspot modes
- Try manual and auto modes

## Questions?

- Create an issue for questions
- Check existing documentation
- Look at closed issues for similar questions

## Recognition

Contributors will be acknowledged in:
- README.md
- Release notes
- Documentation

Thank you for contributing! 🎉
