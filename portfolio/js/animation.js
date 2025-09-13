  // Simple animation for timeline items in About page
        document.addEventListener('DOMContentLoaded', function() {
            const timelineItems = document.querySelectorAll('.timeline-item');
            
            const observer = new IntersectionObserver((entries) => {
                entries.forEach(entry => {
                    if (entry.isIntersecting) {
                        entry.target.style.opacity = 1;
                        entry.target.style.transform = 'translateX(0)';
                    }
                });
            }, {
                threshold: 0.1
            });
            
            timelineItems.forEach(item => {
                item.style.opacity = 0;
                if (item.classList.contains('left')) {
                    item.style.transform = 'translateX(-50px)';
                } else {
                    item.style.transform = 'translateX(50px)';
                }
                item.style.transition = 'all 0.5s ease-in-out';
                observer.observe(item);
            });
            
        });