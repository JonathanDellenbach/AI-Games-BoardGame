flowchart TD
    A[findBestPlacement called] --> B[Check if piece exists]
    B -->|No piece| C[Return -1, -1]
    B -->|Has piece| D[Get all empty positions]
    D --> E[Loop through each position]
    E --> F[Call evaluatePosition for this spot]
    F --> G[Get score back]
    G --> H{Score > best score?}
    H -->|Yes| I[Update best position and score]
    H -->|No| J[Keep current best]
    I --> K{More positions?}
    J --> K
    K -->|Yes| E
    K -->|No| L[Return best position]