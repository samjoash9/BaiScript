// check-icons-simple.js
const fs = require('fs');
const path = require('path');

console.log('Checking icon files in:', __dirname);
console.log('');

const iconFiles = ['icon.ico', 'icon.png', 'icon.icns'];
iconFiles.forEach(file => {
    const filePath = path.join(__dirname, file);
    if (fs.existsSync(filePath)) {
        const stats = fs.statSync(filePath);
        console.log(`✓ ${file}:`);
        console.log(`  - Size: ${stats.size} bytes`);
        console.log(`  - Path: ${filePath}`);

        // Quick check of file headers
        try {
            const buffer = fs.readFileSync(filePath);
            const header = buffer.slice(0, 8).toString('hex');
            console.log(`  - Header (hex): ${header.slice(0, 16)}...`);

            // Basic file type detection
            if (file.endsWith('.ico')) {
                console.log(`  - Type: ICO file (should start with 00000100)`);
            } else if (file.endsWith('.png')) {
                console.log(`  - Type: PNG file (should start with 89504e47)`);
            } else if (file.endsWith('.icns')) {
                console.log(`  - Type: ICNS file (should start with 69636e73)`);
            }
        } catch (err) {
            console.log(`  - Error reading file: ${err.message}`);
        }
    } else {
        console.log(`✗ ${file}: NOT FOUND`);
    }
    console.log('');
});