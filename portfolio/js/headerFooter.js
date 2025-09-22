// Create Header Component
        function createHeader() {
            const headerContainer = document.getElementById('header-container');
            if (!headerContainer) return;
            
            const header = document.createElement('header');
            header.innerHTML = `
                <div class="logo">
                    <i class="fas fa-code"></i>
                    <span>Asiedu Dev. Hub</span>
                </div>
                <nav>
                    <ul>
                        <li><a href="../index.html">Home</a></li>
                        <li><a href="about.html">About</a></li>
                        <li><a href="projects.html">Projects</a></li>
                        <li><a href="skills.html">Skills</a></li>
                        <li><a href="contact.html">Contact</a></li>
                    </ul>
                    <div class="menu-btn">
                        <i class="fas fa-bars"></i>
                    </div>
                </nav>
            `;
            
            headerContainer.appendChild(header);
            
            // Add active class to current page link
            const currentPage = window.location.pathname.split('/').pop();
            const links = header.querySelectorAll('nav a');
            
            links.forEach(link => {
                const linkPage = link.getAttribute('href');
                if (currentPage === linkPage) {
                    link.classList.add('active');
                }
            });
            
            // Mobile menu toggle
            const menuBtn = document.querySelector('.menu-btn');
            const nav = document.querySelector('nav ul');
            
            menuBtn.addEventListener('click', () => {
                nav.style.display = nav.style.display === 'flex' ? 'none' : 'flex';
            });
        }
        
        // Create Footer Component
        function createFooter() {
            const footerContainer = document.getElementById('footer-container');
            if (!footerContainer) return;
            
            const footer = document.createElement('footer');
            footer.innerHTML = `
                <div class="footer-content">
                    <div class="footer-about">
                        <div class="footer-logo">Asiedu Development Hub</div>
                        <p>Creating innovative solutions that connect the physical and digital worlds through web development and embedded systems.</p>
                        <div class="social-links">
                            <a href="https://github.com/AsieduDevelopmentHub" target="_blank"><i class="fab fa-github"></i></a>
                            <a href="https://bitly.cx/SCZZT" target="_blank"><i class="fab fa-linkedin"></i></a>
                            <a href="#" target="_blank"><i class="fab fa-twitter"></i></a>
                            <a href="https://youtube.com/@asiedudev-hub" target="_blank"><i class="fab fa-youtube"></i></a>
                        </div>
                    </div>
                    
                    <div class="footer-links">
                        <h3>Quick Links</h3>
                        <ul>
                            <li><a href="../index.html">Home</a></li>
                            <li><a href="about.html">About</a></li>
                            <li><a href="projects.html">Projects</a></li>
                            <li><a href="skills.html">Skills</a></li>
                            <li><a href="contact.html">Contact</a></li>
                        </ul>
                    </div>
                    
                    <div class="footer-links">
                        <h3>Services</h3>
                        <ul>
                            <li><a href="#">Web Development</a></li>
                            <li><a href="#">IoT Solutions</a></li>
                            <li><a href="#">Embedded Systems</a></li>
                            <li><a href="#">Consultation</a></li>
                            <li><a href="#">Project Guidance</a></li>
                        </ul>
                    </div>
                    
                    <div class="footer-links">
                        <h3>Contact Info</h3>
                        <ul>
                            <li><i class="fas fa-envelope"></i> asiedudev.hub@gmail.com</li>
                            <li><i class="fas fa-phone"></i> +233 555 257 482</li>
                            <li><i class="fas fa-map-marker-alt"></i> Ghana</li>
                        </ul>
                    </div>
                </div>
                
                <div class="copyright">
                    <p>&copy; 2025 Asiedu Development Hub. All rights reserved.</p>
                </div>
            `;
            
            footerContainer.appendChild(footer);
        }
        
        // Initialize components when DOM is loaded
        document.addEventListener('DOMContentLoaded', function() {
            createHeader();
            createFooter();
        });