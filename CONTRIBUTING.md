# Contributing to 3-Axis Gimbal Control System

**⚠️ SECURITY NOTICE**: Before contributing, review [KnownIssues.MD](KnownIssues.MD) to understand current security vulnerabilities and technical debt.

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

---

## Code Review Process

### Before Submitting a PR

1. **Review Known Issues**: Check [KnownIssues.MD](KnownIssues.MD) to ensure you're not duplicating work
2. **Run Tests**: Follow [docs/TESTING.md](docs/TESTING.md) manual testing checklist
3. **Security Check**: Ensure your changes don't introduce new security issues
4. **Code Quality**: Follow the coding guidelines below

### PR Review Checklist

**Security** (Critical):
- [ ] No hardcoded credentials or secrets
- [ ] No SQL injection or command injection vulnerabilities
- [ ] Input validation on all user inputs
- [ ] Authentication/authorization properly implemented
- [ ] No plain text password storage (unless encrypted)
- [ ] CORS properly configured (if backend changes)
- [ ] Rate limiting considered for API endpoints

**Code Quality**:
- [ ] Follows existing code style
- [ ] No compiler warnings
- [ ] Code is commented where necessary
- [ ] No magic numbers (use named constants)
- [ ] Error handling implemented
- [ ] Memory leaks checked (for C++)
- [ ] Thread safety considered (if using mutexes)

**Testing**:
- [ ] Manual tests performed (see [docs/TESTING.md](docs/TESTING.md))
- [ ] Automated tests added (if applicable)
- [ ] No test regressions
- [ ] Edge cases covered

**Documentation**:
- [ ] README updated (if user-facing changes)
- [ ] API docs updated (if API changes)
- [ ] Comments added for complex logic
- [ ] CHANGELOG.md updated
- [ ] KnownIssues.MD updated (if fixing an issue or introducing a known limitation)

**Hardware Safety** (if applicable):
- [ ] Servo limits enforced (0-180°)
- [ ] No rapid movements that could damage hardware
- [ ] Safe defaults maintained
- [ ] Graceful degradation on errors

### Code Review Response Time

- Initial review: Within 3-5 business days
- Follow-up reviews: Within 2 business days
- Urgent security fixes: Within 24 hours

---

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
