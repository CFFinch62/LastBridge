#!/bin/bash

# Test script to verify window fits on 1366x768 laptop screen
echo "Testing LAST application window fit for 1366x768 laptop screen..."

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
            
            # Get window geometry including position
            GEOMETRY_INFO=$(xwininfo -id $WINDOW_ID)
            WIDTH=$(echo "$GEOMETRY_INFO" | grep "Width:" | awk '{print $2}')
            HEIGHT=$(echo "$GEOMETRY_INFO" | grep "Height:" | awk '{print $2}')
            ABS_X=$(echo "$GEOMETRY_INFO" | grep "Absolute upper-left X:" | awk '{print $4}')
            ABS_Y=$(echo "$GEOMETRY_INFO" | grep "Absolute upper-left Y:" | awk '{print $4}')
            
            echo "Window dimensions: ${WIDTH}x${HEIGHT}"
            echo "Window position: (${ABS_X}, ${ABS_Y})"
            
            # Calculate bottom-right corner
            BOTTOM_X=$((ABS_X + WIDTH))
            BOTTOM_Y=$((ABS_Y + HEIGHT))
            echo "Window bottom-right corner: (${BOTTOM_X}, ${BOTTOM_Y})"
            
            # Check if window fits within 1366x768 screen
            LAPTOP_WIDTH=1366
            LAPTOP_HEIGHT=768
            
            echo "Laptop screen resolution: ${LAPTOP_WIDTH}x${LAPTOP_HEIGHT}"
            
            # Check horizontal fit
            if [ "$BOTTOM_X" -le "$LAPTOP_WIDTH" ]; then
                echo "✓ Window fits horizontally within laptop screen"
            else
                echo "✗ Window extends beyond laptop screen width"
                echo "  Window right edge: $BOTTOM_X, Screen width: $LAPTOP_WIDTH"
            fi
            
            # Check vertical fit (most important for your issue)
            if [ "$BOTTOM_Y" -le "$LAPTOP_HEIGHT" ]; then
                echo "✓ Window fits vertically within laptop screen"
            else
                echo "✗ Window extends beyond laptop screen height"
                echo "  Window bottom edge: $BOTTOM_Y, Screen height: $LAPTOP_HEIGHT"
                echo "  Overflow: $((BOTTOM_Y - LAPTOP_HEIGHT)) pixels"
            fi
            
            # Check total height constraint
            if [ "$HEIGHT" -le 720 ]; then
                echo "✓ Window total height (${HEIGHT}px) is within 720px constraint"
            else
                echo "✗ Window total height (${HEIGHT}px) exceeds 720px constraint"
            fi
            
            # Check for maximize button (this is harder to detect programmatically)
            echo "Note: Please manually verify that the maximize button is present in the title bar"
            
        else
            echo "✗ Could not find LAST window"
        fi
    else
        echo "⚠ xwininfo not available - cannot check window dimensions"
    fi
else
    echo "✗ LAST application is not running"
fi

echo "Laptop fit test completed."
