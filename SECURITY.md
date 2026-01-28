# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |

## Security Updates

### Version 1.0.1 (2024-01-28)

**Critical Security Fixes:**

1. **FastAPI ReDoS Vulnerability**
   - **Issue**: Duplicate Advisory: FastAPI Content-Type Header ReDoS
   - **Affected**: FastAPI <= 0.109.0
   - **Fixed**: Upgraded to FastAPI 0.109.1
   - **Impact**: High - Remote attackers could cause denial of service via crafted Content-Type headers

2. **python-multipart Multiple Vulnerabilities**
   - **Issue 1**: Arbitrary File Write via Non-Default Configuration
     - **Affected**: python-multipart < 0.0.22
     - **Fixed**: Upgraded to 0.0.22
     - **Impact**: Critical - Could allow arbitrary file writes
   
   - **Issue 2**: Denial of Service via malformed multipart/form-data
     - **Affected**: python-multipart < 0.0.18
     - **Fixed**: Upgraded to 0.0.22
     - **Impact**: High - Could cause service disruption
   
   - **Issue 3**: Content-Type Header ReDoS
     - **Affected**: python-multipart <= 0.0.6
     - **Fixed**: Upgraded to 0.0.22
     - **Impact**: High - Could cause CPU exhaustion

**Action Required:**
- Update to version 1.0.1 or later
- Run: `pip install -r requirements.txt --upgrade`

## Reporting a Vulnerability

If you discover a security vulnerability, please report it by:

1. **Do NOT** open a public issue
2. Email security concerns to the maintainers
3. Include:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

We will respond within 48 hours and provide a timeline for a fix.

## Security Best Practices

### For Production Deployment

1. **Update Dependencies Regularly**
   ```bash
   pip list --outdated
   pip install -r requirements.txt --upgrade
   ```

2. **Change Default Credentials**
   ```cpp
   // esp32_firmware/include/config.h
   #define WIFI_PASSWORD "your-secure-password"
   #define HOTSPOT_PASSWORD "your-secure-hotspot-password"
   ```

3. **Enable HTTPS/WSS**
   - Use reverse proxy (nginx) with SSL/TLS
   - Configure Let's Encrypt certificates
   - Enforce HTTPS redirects

4. **Implement Authentication**
   - Enable JWT token authentication in FastAPI
   - Use strong secret keys
   - Implement rate limiting

5. **Network Security**
   - Use firewall rules
   - Restrict API access to known IPs
   - Disable unnecessary ports
   - Use VPN for remote access

6. **Regular Security Audits**
   ```bash
   # Check for known vulnerabilities
   pip install safety
   safety check -r requirements.txt
   
   # Or use pip-audit
   pip install pip-audit
   pip-audit
   ```

7. **Monitor Logs**
   - Enable logging in production
   - Monitor for suspicious activity
   - Set up alerts for errors

8. **Backup Configuration**
   - Regular backups of configuration
   - Secure storage of credentials
   - Document recovery procedures

## Security Checklist

Before deploying to production:

- [ ] Update all dependencies to latest secure versions
- [ ] Change default WiFi passwords
- [ ] Change default hotspot password
- [ ] Enable HTTPS/SSL
- [ ] Implement authentication
- [ ] Configure firewall rules
- [ ] Enable security headers
- [ ] Implement rate limiting
- [ ] Set up monitoring and alerts
- [ ] Configure regular backups
- [ ] Review and test security measures
- [ ] Document security procedures

## Known Security Considerations

### ESP32 Firmware

1. **WiFi Credentials**: Stored in plain text in config.h
   - **Mitigation**: Use secure WiFi passwords, consider encryption for sensitive deployments

2. **Hotspot Mode**: Open authentication by default
   - **Mitigation**: Strong password set, consider WPA2-Enterprise for production

3. **No HTTPS on ESP32**: Web server uses HTTP
   - **Mitigation**: Use backend with HTTPS, or deploy ESP32 on isolated network

### Backend API

1. **CORS**: Enabled for all origins by default
   - **Mitigation**: Restrict to specific origins in production

2. **No Authentication**: API is open by default
   - **Mitigation**: Enable authentication middleware for production

3. **WebSocket**: No authentication on WebSocket connections
   - **Mitigation**: Implement token-based WebSocket authentication

## Security Updates Policy

- Security patches are released as soon as possible
- Critical vulnerabilities: Within 24-48 hours
- High severity: Within 1 week
- Medium/Low severity: Next regular release

## Dependencies Security

We regularly monitor our dependencies for security vulnerabilities using:
- GitHub Security Advisories
- Safety (Python security scanner)
- pip-audit
- Dependabot alerts

## Contact

For security concerns, please contact the project maintainers through:
- GitHub Security Advisories (preferred)
- GitHub Issues (for non-sensitive issues)
- Email (for sensitive security issues)

## Acknowledgments

We appreciate responsible disclosure of security vulnerabilities. Contributors who report valid security issues will be acknowledged in release notes (unless they prefer to remain anonymous).

---

**Last Updated**: 2024-01-28  
**Version**: 1.0.1
