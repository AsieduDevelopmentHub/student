import React from 'react';
import './Footer.scss';

const Footer = () => {
  return (
    <footer className="footer">
      <div className="footer__content">
        <div className="footer__about">
          <div className="footer__logo">Asiedu Development Hub</div>
          <p>Creating innovative solutions that connect the physical and digital worlds through web development and embedded systems.</p>
          <div className="footer__social">
            <a href="https://github.com/AsieduDevelopmentHub" target="_blank"><i className="fab fa-github"></i></a>
            <a href="https://bitly.cx/SCZZT" target="_blank"><i className="fab fa-linkedin"></i></a>
            <a href="#" target="_blank"><i className="fab fa-twitter"></i></a>
            <a href="https://youtube.com/@asiedudev-hub" target="_blank"><i className="fab fa-youtube"></i></a>
          </div>
        </div>
        
        <div className="footer__links">
          <h3>Quick Links</h3>
          <ul>
            <li><a href="index.html">Home</a></li>
            <li><a href="about.html">About</a></li>
            <li><a href="projects.html">Projects</a></li>
            <li><a href="skills.html">Skills</a></li>
            <li><a href="contact.html">Contact</a></li>
          </ul>
        </div>
        
        <div className="footer__links">
          <h3>Services</h3>
          <ul>
            <li><a href="#">Web Development</a></li>
            <li><a href="#">IoT Solutions</a></li>
            <li><a href="#">Embedded Systems</a></li>
            <li><a href="#">Consultation</a></li>
            <li><a href="#">Project Guidance</a></li>
          </ul>
        </div>
        
        <div className="footer__links">
          <h3>Contact Info</h3>
          <ul>
            <li><i className="fas fa-envelope"></i> asiedudev.hub@gmail.com</li>
            <li><i className="fas fa-phone"></i> +233 555 257 482</li>
            <li><i className="fas fa-map-marker-alt"></i> Ghana</li>
          </ul>
        </div>
      </div>
      
      <div className="footer__copyright">
        <p>&copy; 2025 Asiedu Development Hub. All rights reserved.</p>
      </div>
    </footer>
  );
};

export default Footer;