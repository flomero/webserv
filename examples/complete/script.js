// Canvas and Context Setup
const canvas = document.getElementById('matrixCanvas');
const ctx = canvas.getContext('2d');

// Matrix Effect Variables
const fontSize = 16;
let columns = Math.floor(canvas.width / fontSize);
let drops = Array(columns).fill(1);

// Resize Canvas to Fullscreen and Update Matrix Variables
function resizeCanvas() {
	canvas.width = window.innerWidth;
	canvas.height = window.innerHeight;
	columns = Math.floor(canvas.width / fontSize);
	drops = Array(columns).fill(1);
}

resizeCanvas();
window.addEventListener('resize', resizeCanvas);

// Expanded Character Set
const letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:\'",.<>?/`~';

// Matrix Drawing Function
function drawMatrix() {
	ctx.fillStyle = 'rgba(0, 0, 0, 0.05)';
	ctx.fillRect(0, 0, canvas.width, canvas.height);
	ctx.font = `${fontSize}px monospace`;

	for (let i = 0; i < drops.length; i++) {
		const text = letters.charAt(Math.floor(Math.random() * letters.length));
		const x = i * fontSize;
		const y = drops[i] * fontSize;

		// Default Dark Green Text
		ctx.fillStyle = '#003300';
		ctx.fillText(text, x, y);

		// Reset if Characters Reach Bottom
		if (y > canvas.height && Math.random() > 0.975) {
			drops[i] = 0;
		}

		drops[i]++;
	}

	requestAnimationFrame(drawMatrix);
}

// Start Matrix Animation
drawMatrix();

// Header Glitch Effect
const glitchElements = document.querySelectorAll('.glitch');

// Random Glitch Effect for Elements
function addGlitchEffect() {
	glitchElements.forEach(element => {
		const glitchChance = Math.random();
		if (glitchChance > 0.8) {
			element.style.opacity = Math.random() > 0.5 ? '0.8' : '1';

			// Change individual letters
			const originalText = element.textContent;
			let glitchedText = '';
			for (let i = 0; i < originalText.length; i++) {
				if (Math.random() > 0.7) {
					glitchedText += letters.charAt(Math.floor(Math.random() * letters.length));
				} else {
					glitchedText += originalText[i];
				}
			}
			element.textContent = glitchedText;

			// Revert back to original text after a short delay
			setTimeout(() => {
				element.textContent = originalText;
			}, 200);
		} else {
			element.style.opacity = '1';
		}
	});
}

// Apply Glitch Effect at Random Intervals
setInterval(addGlitchEffect, 500);
