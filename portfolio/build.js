const fs = require('fs');
const path = require('path');

// Component import function
function importComponent(componentPath) {
  try {
    return fs.readFileSync(path.join(__dirname, componentPath), 'utf8');
  } catch (error) {
    console.error(`Error reading component ${componentPath}:`, error);
    return '';
  }
}

// Create pages
const pages = [
  { name: 'index', title: 'Home - Asiedu Development Hub' },
  { name: 'about', title: 'About - Asiedu Development Hub' },
  { name: 'projects', title: 'Projects - Asiedu Development Hub' },
  { name: 'skills', title: 'Skills - Asiedu Development Hub' },
  { name: 'contact', title: 'Contact - Asiedu Development Hub' },
];

// Create dist directory if it doesn't exist
if (!fs.existsSync('dist')) {
  fs.mkdirSync('dist');
}

// Build each page
pages.forEach(page => {
  const header = importComponent('src/components/Header/Header.html');
  const footer = importComponent('src/components/Footer/Footer.html');
  const content = importComponent(`src/pages/${page.name}.html`);
  
  const html = `
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>${page.title}</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="styles.css">
</head>
<body>
  ${header}
  <main>
    ${content}
  </main>
  ${footer}
  <script src="script.js"></script>
</body>
</html>
  `;
  
  fs.writeFileSync(`dist/${page.name}.html`, html);
  console.log(`Built ${page.name}.html`);
});

// Copy CSS and JS
fs.copyFileSync('src/styles/responsive.scss', 'dist/styles.css');
console.log('Copied styles.css');

// Create a simple JS file for interactive elements
const jsContent = `
// Mobile menu toggle
document.addEventListener('DOMContentLoaded', function() {
  const menuBtn = document.querySelector('.header__menu-btn');
  const nav = document.querySelector('.header__nav ul');
  
  if (menuBtn && nav) {
    menuBtn.addEventListener('click', function() {
      nav.classList.toggle('show');
    });
  }
  
  // Set active navigation link
  const currentPage = window.location.pathname.split('/').pop();
  const navLinks = document.querySelectorAll('.header__nav a');
  
  navLinks.forEach(link => {
    const linkPage = link.getAttribute('href');
    if (currentPage === linkPage) {
      link.classList.add('active');
    }
  });
});
`;

fs.writeFileSync('dist/script.js', jsContent);
console.log('Created script.js');

console.log('Build completed!');