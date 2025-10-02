#!/bin/bash

# Test script to verify window resizing behavior
echo "Testing LAST application window resizing..."

# Check if the application is running
if pgrep -f "./last" > /dev/null; then
    echo "✓ LAST application is running"
    
    # Get window information using xwininfo and wmctrl
    if command -v xwininfo &> /dev/null && command -v wmctrl &> /dev/null; then
        echo "Getting window information..."
        
        # Find the LAST window
        WINDOW_ID=$(xwininfo -name "LAST - Linux Advanced Serial Terminal" 2>/dev/null | grep "Window id:" | cut -d' ' -f4)
        
        if [ ! -z "$WINDOW_ID" ]; then
            echo "✓ Found LAST window (ID: $WINDOW_ID)"
            
            # Get initial dimensions
            INITIAL_GEOMETRY=$(xwininfo -id $WINDOW_ID | grep -E "Width:|Height:")
            INITIAL_WIDTH=$(echo "$INITIAL_GEOMETRY" | grep "Width:" | awk '{print $2}')
            INITIAL_HEIGHT=$(echo "$INITIAL_GEOMETRY" | grep "Height:" | awk '{print $2}')
            echo "Initial dimensions: ${INITIAL_WIDTH}x${INITIAL_HEIGHT}"
            
            # Try to resize to a larger size (1600x900)
            echo "Attempting to resize to 1600x900..."
            wmctrl -i -r $WINDOW_ID -e 0,-1,-1,1600,900
            sleep 1
            
            # Check new dimensions
            NEW_GEOMETRY=$(xwininfo -id $WINDOW_ID | grep -E "Width:|Height:")
            NEW_WIDTH=$(echo "$NEW_GEOMETRY" | grep "Width:" | awk '{print $2}')
            NEW_HEIGHT=$(echo "$NEW_GEOMETRY" | grep "Height:" | awk '{print $2}')
            echo "After resize attempt: ${NEW_WIDTH}x${NEW_HEIGHT}"
            
            if [ "$NEW_WIDTH" -gt "$INITIAL_WIDTH" ] && [ "$NEW_HEIGHT" -gt "$INITIAL_HEIGHT" ]; then
                echo "✓ Window successfully resized to larger dimensions"
            else
                echo "⚠ Window did not resize as expected"
            fi
            
            # Try to resize to smaller than minimum (1000x600)
            echo "Attempting to resize below minimum (1000x600)..."
            wmctrl -i -r $WINDOW_ID -e 0,-1,-1,1000,600
            sleep 1
            
            # Check dimensions after small resize attempt
            SMALL_GEOMETRY=$(xwininfo -id $WINDOW_ID | grep -E "Width:|Height:")
            SMALL_WIDTH=$(echo "$SMALL_GEOMETRY" | grep "Width:" | awk '{print $2}')
            SMALL_HEIGHT=$(echo "$SMALL_GEOMETRY" | grep "Height:" | awk '{print $2}')
            echo "After small resize attempt: ${SMALL_WIDTH}x${SMALL_HEIGHT}"
            
            if [ "$SMALL_WIDTH" -ge 1280 ] && [ "$SMALL_HEIGHT" -ge 720 ]; then
                echo "✓ Window correctly maintained minimum size constraints"
            else
                echo "✗ Window was resized below minimum constraints"
            fi
            
        else
            echo "✗ Could not find LAST window"
        fi
    else
        echo "⚠ xwininfo or wmctrl not available - cannot test resizing"
    fi
else
    echo "✗ LAST application is not running"
fi

echo "Resize test completed."
