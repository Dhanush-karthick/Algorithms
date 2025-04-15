import java.util.*;

class LetterCombinations {
    // Mapping of digits to letters
    private static final String[] DIGIT_MAP = {
        "",     // 0
        "",     // 1
        "abc",  // 2
        "def",  // 3
        "ghi",  // 4
        "jkl",  // 5
        "mno",  // 6
        "pqrs", // 7
        "tuv",  // 8
        "wxyz"  // 9
    };
    
    public List<String> letterCombinations(String digits) {
        List<String> result = new ArrayList<>();
        
        // Handle empty input
        if (digits == null || digits.length() == 0) {
            return result;
        }
        
        // Start backtracking
        backtrack(digits, 0, new StringBuilder(), result);
        
        return result;
    }
    
    private void backtrack(String digits, int index, StringBuilder current, List<String> result) {
        // If we've processed all digits, add the current combination
        if (index == digits.length()) {
            result.add(current.toString());
            return;
        }
        
        // Get the letters for the current digit
        String letters = DIGIT_MAP[digits.charAt(index) - '0'];
        
        // Try each letter for the current digit
        for (char letter : letters.toCharArray()) {
            // Add the letter to current combination
            current.append(letter);
            
            // Recurse to next digit
            backtrack(digits, index + 1, current, result);
            
            // Backtrack by removing the last added letter
            current.deleteCharAt(current.length() - 1);
        }
    }
    
    // Main method to test the implementation
    public static void main(String[] args) {
        LetterCombinations solution = new LetterCombinations();
        
        // Test cases
        String[] testCases = {"23", "79", "2", ""};
        
        for (String digits : testCases) {
            List<String> combinations = solution.letterCombinations(digits);
            System.out.println("Input: " + digits);
            System.out.println("Combinations: " + combinations);
            System.out.println();
        }
    }
}
