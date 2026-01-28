# Deployment Guide

## Overview

This guide covers deployment options for the ESP32 3-Axis Gimbal Control System.

## Deployment Options

### 1. Standalone ESP32 (Recommended for Most Users)

**Best for:**
- Single gimbal operation
- Local network use
- Simplest setup

**Deployment Steps:**

1. **Flash Firmware**
   ```bash
   cd esp32_firmware
   pio run --target upload
   ```

2. **Configure Network**
   - Edit `include/config.h` with your WiFi credentials
   - Or use hotspot mode (no configuration needed)

3. **Power On**
   - ESP32 attempts WiFi connection
   - Falls back to hotspot if connection fails
   - Web interface available immediately

4. **Access**
   - WiFi mode: `http://[ESP32_IP]`
   - Hotspot mode: `http://192.168.4.1`

**Pros:**
- No additional server needed
- Works offline
- Simple setup

**Cons:**
- Limited to single gimbal
- Basic API features
- No persistent data storage

### 2. With FastAPI Backend

**Best for:**
- Multiple gimbals
- Advanced features (presets, logging)
- Remote access
- Mobile app support

**Deployment Steps:**

1. **Deploy Backend Server**
   ```bash
   cd backend
   pip install -r requirements.txt
   python main.py
   ```

2. **Configure ESP32**
   - Flash firmware as usual
   - ESP32 can work independently
   - Backend communicates via REST/WebSocket

3. **Access Points**
   - ESP32 Web UI: `http://[ESP32_IP]`
   - Backend API: `http://[SERVER_IP]:8000`
   - API Docs: `http://[SERVER_IP]:8000/docs`

**Pros:**
- Advanced features
- Multiple device support
- Better authentication
- Data persistence

**Cons:**
- Requires separate server
- More complex setup
- Additional maintenance

### 3. Cloud Deployment (Advanced)

**Best for:**
- Remote access from anywhere
- Multiple users
- Professional applications
- Data analytics

See [Cloud Deployment](#cloud-deployment) section below.

## Production Deployment

### ESP32 Configuration

1. **Update WiFi Credentials**
   ```cpp
   // config.h
   #define WIFI_SSID "YourProductionSSID"
   #define WIFI_PASSWORD "SecurePassword123!"
   ```

2. **Set Production Hotspot Password**
   ```cpp
   #define HOTSPOT_PASSWORD "SecureHotspotPass123"
   ```

3. **Optimize Performance**
   ```cpp
   #define SENSOR_UPDATE_RATE 10    // Adjust as needed
   #define SERVO_UPDATE_RATE 20
   #define WEBSOCKET_UPDATE_RATE 100
   ```

4. **Disable Debug Output** (Optional)
   - Comment out Serial.println statements
   - Or set build flag: `-DCORE_DEBUG_LEVEL=0`

### FastAPI Backend Deployment

#### Option A: Linux Server (Ubuntu/Debian)

1. **Install Dependencies**
   ```bash
   sudo apt update
   sudo apt install python3 python3-pip python3-venv
   ```

2. **Setup Application**
   ```bash
   cd backend
   python3 -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt
   ```

3. **Create Systemd Service**
   ```bash
   sudo nano /etc/systemd/system/gimbal-api.service
   ```

   ```ini
   [Unit]
   Description=Gimbal Control API
   After=network.target

   [Service]
   Type=simple
   User=www-data
   WorkingDirectory=/opt/gimbal/backend
   Environment="PATH=/opt/gimbal/backend/venv/bin"
   ExecStart=/opt/gimbal/backend/venv/bin/uvicorn main:app --host 0.0.0.0 --port 8000
   Restart=always

   [Install]
   WantedBy=multi-user.target
   ```

4. **Enable and Start**
   ```bash
   sudo systemctl enable gimbal-api
   sudo systemctl start gimbal-api
   sudo systemctl status gimbal-api
   ```

#### Option B: Docker Deployment

1. **Create Dockerfile**
   ```dockerfile
   FROM python:3.11-slim

   WORKDIR /app

   COPY requirements.txt .
   RUN pip install --no-cache-dir -r requirements.txt

   COPY . .

   EXPOSE 8000

   CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
   ```

2. **Build Image**
   ```bash
   docker build -t gimbal-api .
   ```

3. **Run Container**
   ```bash
   docker run -d \
     --name gimbal-api \
     --restart unless-stopped \
     -p 8000:8000 \
     gimbal-api
   ```

#### Option C: Docker Compose

1. **Create docker-compose.yml**
   ```yaml
   version: '3.8'

   services:
     api:
       build: ./backend
       ports:
         - "8000:8000"
       restart: unless-stopped
       environment:
         - ENVIRONMENT=production
       volumes:
         - ./data:/app/data
   ```

2. **Deploy**
   ```bash
   docker-compose up -d
   ```

### Reverse Proxy (Nginx)

For production, use Nginx as reverse proxy:

1. **Install Nginx**
   ```bash
   sudo apt install nginx
   ```

2. **Configure**
   ```nginx
   # /etc/nginx/sites-available/gimbal
   server {
       listen 80;
       server_name gimbal.yourdomain.com;

       location / {
           proxy_pass http://localhost:8000;
           proxy_http_version 1.1;
           proxy_set_header Upgrade $http_upgrade;
           proxy_set_header Connection "upgrade";
           proxy_set_header Host $host;
           proxy_set_header X-Real-IP $remote_addr;
       }
   }
   ```

3. **Enable Site**
   ```bash
   sudo ln -s /etc/nginx/sites-available/gimbal /etc/nginx/sites-enabled/
   sudo nginx -t
   sudo systemctl reload nginx
   ```

### SSL/TLS (Let's Encrypt)

1. **Install Certbot**
   ```bash
   sudo apt install certbot python3-certbot-nginx
   ```

2. **Obtain Certificate**
   ```bash
   sudo certbot --nginx -d gimbal.yourdomain.com
   ```

3. **Auto-Renewal**
   ```bash
   sudo certbot renew --dry-run
   ```

## Cloud Deployment

### AWS Deployment

#### EC2 Instance

1. **Launch Instance**
   - Ubuntu 22.04 LTS
   - t2.micro (free tier) or larger
   - Configure security group (ports 80, 443, 8000)

2. **Deploy Application**
   ```bash
   # Connect to instance
   ssh ubuntu@your-instance-ip

   # Install dependencies
   sudo apt update
   sudo apt install python3-pip python3-venv nginx

   # Clone and setup
   git clone [your-repo]
   cd backend
   python3 -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt

   # Setup systemd service (see above)
   ```

3. **Configure Domain**
   - Point DNS to EC2 public IP
   - Setup SSL with Let's Encrypt

#### AWS Lambda (Serverless)

Use Mangum adapter for FastAPI on Lambda:

```python
from mangum import Mangum
from main import app

handler = Mangum(app)
```

### Google Cloud Platform

#### Cloud Run

1. **Create Dockerfile** (see above)

2. **Deploy**
   ```bash
   gcloud run deploy gimbal-api \
     --source . \
     --platform managed \
     --region us-central1 \
     --allow-unauthenticated
   ```

### DigitalOcean

#### App Platform

1. **Create app.yaml**
   ```yaml
   name: gimbal-api
   services:
   - name: api
     github:
       repo: your-username/your-repo
       branch: main
       deploy_on_push: true
     source_dir: /backend
     run_command: uvicorn main:app --host 0.0.0.0 --port 8000
     http_port: 8000
   ```

2. **Deploy via Web Interface**
   - Connect GitHub repository
   - Configure environment variables
   - Deploy

## Environment Variables

For production, use environment variables instead of hardcoded values:

**.env file:**
```bash
# Backend configuration
ENVIRONMENT=production
SECRET_KEY=your-secret-key-here
API_KEY=your-api-key-here

# Database (if using)
DATABASE_URL=postgresql://user:pass@localhost/gimbal

# CORS origins
ALLOWED_ORIGINS=https://yourdomain.com,https://app.yourdomain.com
```

**Load in FastAPI:**
```python
from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    environment: str = "development"
    secret_key: str
    api_key: str
    
    class Config:
        env_file = ".env"

settings = Settings()
```

## Monitoring

### Application Monitoring

1. **Logging**
   ```python
   import logging
   
   logging.basicConfig(
       level=logging.INFO,
       format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
   )
   ```

2. **Health Checks**
   - Use `/api/health` endpoint
   - Monitor response time
   - Check sensor connectivity

3. **Metrics**
   - Request count
   - Response time
   - Error rate
   - WebSocket connections

### Infrastructure Monitoring

- **Uptime monitoring**: UptimeRobot, Pingdom
- **APM**: New Relic, Datadog
- **Logs**: CloudWatch, Loggly, Papertrail

## Backup & Recovery

### Configuration Backup

1. **ESP32 Settings**
   - Export via API
   - Store in version control
   - Document pin configurations

2. **Backend Data**
   ```bash
   # Backup presets
   curl http://localhost:8000/api/presets > presets_backup.json
   ```

3. **Database** (if applicable)
   ```bash
   pg_dump gimbal_db > backup.sql
   ```

### Disaster Recovery

1. **ESP32 Failure**
   - Flash backup firmware
   - Restore configuration
   - Test sensors and servos

2. **Backend Failure**
   - Restart service
   - Restore from backup
   - Verify data integrity

## Scaling

### Horizontal Scaling

**Load Balancer + Multiple Backend Instances:**

```nginx
upstream gimbal_backend {
    server backend1:8000;
    server backend2:8000;
    server backend3:8000;
}

server {
    location / {
        proxy_pass http://gimbal_backend;
    }
}
```

### Vertical Scaling

- Increase server resources (CPU, RAM)
- Optimize database queries
- Cache frequently accessed data

## Security Checklist

- [ ] Change all default passwords
- [ ] Enable HTTPS/WSS
- [ ] Implement authentication
- [ ] Set up firewall rules
- [ ] Regular security updates
- [ ] Monitor logs for suspicious activity
- [ ] Backup regularly
- [ ] Use environment variables for secrets
- [ ] Implement rate limiting
- [ ] Enable CORS only for trusted origins

## Maintenance

### Regular Tasks

**Daily:**
- Check logs for errors
- Monitor system health

**Weekly:**
- Review security logs
- Check for firmware updates
- Verify backups

**Monthly:**
- Update dependencies
- Review performance metrics
- Test disaster recovery

### Updates

**Firmware Updates:**
```bash
cd esp32_firmware
git pull
pio run --target upload
```

**Backend Updates:**
```bash
cd backend
git pull
pip install -r requirements.txt --upgrade
sudo systemctl restart gimbal-api
```

## Troubleshooting Production Issues

### ESP32 Not Responding
1. Check power supply
2. Verify network connectivity
3. Check serial logs
4. Reflash firmware if needed

### Backend Service Down
1. Check service status: `sudo systemctl status gimbal-api`
2. Review logs: `sudo journalctl -u gimbal-api -n 100`
3. Restart service: `sudo systemctl restart gimbal-api`

### High Latency
1. Check network bandwidth
2. Review server load
3. Optimize database queries
4. Scale horizontally if needed

## Support & Resources

- Documentation: `/docs` folder
- API Reference: `http://[server]/docs`
- GitHub Issues: [Your Repository]
- Community Forum: [If available]

## Appendix

### Production Checklist

- [ ] Firmware flashed with production config
- [ ] WiFi credentials configured
- [ ] Hotspot password changed
- [ ] Backend deployed and running
- [ ] SSL/TLS configured
- [ ] Monitoring setup
- [ ] Backups configured
- [ ] Security hardening complete
- [ ] Load testing performed
- [ ] Documentation updated
- [ ] Team trained on operations
