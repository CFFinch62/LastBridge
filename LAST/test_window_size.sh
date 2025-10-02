#!/bin/bash

# Test script to verify window sizing
echo "Testing LAST application window sizing..."

# Check if the application is running
if pgrep -f "./last" > /dev/null; then
    echo "✓ LAST application is running"
    
    # Get window information using xwininfo
    if command -v xwininfo &> /dev/null; then
        echo "Getting window information..."
        # Find the LAST window
        WINDOW_ID=$(xwininfo -name "LAST - Linux Advanced Serial Terminal" 2>/dev/null | grep "Window id:" | cut -d' ' -f4)
        
        if [ ! -z "$WINDOW_ID" ]; then
            echo "✓ Found LAST window (ID: $WINDOW_ID)"
            
            # Get window geometry
            GEOMETRY=$(xwininfo -id $WINDOW_ID | grep -E "Width:|Height:")
            echo "Window dimensions:"
            echo "$GEOMETRY"
            
            # Extract width and height
            WIDTH=$(echo "$GEOMETRY" | grep "Width:" | awk '{print $2}')
            HEIGHT=$(echo "$GEOMETRY" | grep "Height:" | awk '{print $2}')
            
            echo "Parsed dimensions: ${WIDTH}x${HEIGHT}"
            
            # Check if dimensions meet requirements
            if [ "$WIDTH" -ge 1280 ] && [ "$HEIGHT" -ge 720 ]; then
                echo "✓ Window size meets minimum requirements (1280x720)"
            else
                echo "✗ Window size does not meet minimum requirements"
                echo "  Expected: >= 1280x720"
                echo "  Actual: ${WIDTH}x${HEIGHT}"
            fi
            
            # Check aspect ratio (should be close to 16:9 = 1.777...)
            ASPECT_RATIO=$(echo "scale=3; $WIDTH / $HEIGHT" | bc -l 2>/dev/null || echo "N/A")
            echo "Aspect ratio: $ASPECT_RATIO (16:9 ≈ 1.778)"
            
        else
            echo "✗ Could not find LAST window"
        fi
    else
        echo "⚠ xwininfo not available - cannot check window dimensions"
    fi
else
    echo "✗ LAST application is not running"
fi

echo "Test completed."
