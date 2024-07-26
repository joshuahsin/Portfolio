# Leetcode Problems (7/1 - 10/3)
<img width="768" alt="Screen Shot 2022-10-03 at 4 17 39 PM" src="https://user-images.githubusercontent.com/32546754/193702553-92e9889d-a45d-4e3b-8027-62daf8aaae32.png">


# Hard Problems
## Median of Two Sorted Arrays
<img width="423" alt="Screen Shot 2022-10-03 at 4 56 26 PM" src="https://user-images.githubusercontent.com/32546754/193706201-2c9b5225-ba7c-4690-8c32-c2682d1812b9.png">

### Constraints
<img width="334" alt="Screen Shot 2022-10-03 at 4 56 43 PM" src="https://user-images.githubusercontent.com/32546754/193706227-62af9c88-4afa-4d64-8fa0-a1eb1f009ac0.png">

### Examples
<img width="455" alt="Screen Shot 2022-10-03 at 4 56 59 PM" src="https://user-images.githubusercontent.com/32546754/193706255-5543e388-8d39-4829-8ba0-06eb4dc55b5b.png">

### Solution in C
<img width="700" alt="Screen Shot 2022-10-03 at 5 00 10 PM" src="https://user-images.githubusercontent.com/32546754/193706558-51204a76-ac68-4a20-bdcd-9bb4f9cec3d3.png">

To solve this problem, I first created an integer array of the size of the floor of half the sum of the size of both arrays + 1. I only made the array half the size of the total merged array 
since I only needed the middle value/values of the larger sorted array. I then initialized two integer values representing the current index of both arrays. After this, I iterated through a 
loop the same amount of times as the size of the array I created, adding  the current smallest value between the two arrays to the array and incrementing the corresponding array index each 
time. Following this, if the total merged array had an odd length, I just returned value of the last index of the half array, as that would be the only middle value. If the total merged array  
had an even length, I took the average of the last two values in the half array, as they would be the two middle values.

### Time Complexity
Everything before the loop is constant O(1). The loop runs in O(m + n / 2) time if m is equal to the size of nums1 array and n is equal to the size of nums2 array. After the loop, everything 
is also constant time. Therefore, the Overall time complexity is O(m + n / 2).

### Space Complexity
The overall space complexity is O(m + n / 2).

<br>

## Merge k Sorted Lists
<img width="440" alt="Screen Shot 2022-10-03 at 8 04 14 PM" src="https://user-images.githubusercontent.com/32546754/193725624-85cb9189-9a05-42d3-80b9-318b60464509.png">

### Constraints
<img width="366" alt="Screen Shot 2022-10-03 at 8 05 19 PM" src="https://user-images.githubusercontent.com/32546754/193725739-5e81bf52-07da-4eb2-a213-ea1f7ca7a441.png">

### Examples
<img width="440" alt="Screen Shot 2022-10-03 at 8 05 41 PM" src="https://user-images.githubusercontent.com/32546754/193725782-326df080-7971-4ed3-b4e2-e2471ed43d56.png">

### Solution in C++
<img width="509" alt="Screen Shot 2022-10-03 at 8 07 02 PM" src="https://user-images.githubusercontent.com/32546754/193725915-1551def1-b483-4c24-978d-ba3d64354d52.png">

To solve this problem, first iterate through the vector of LinkedLists. For every value in each LinikedList, insert the value into a vector of integers. If the vector is empty, return NULL. 
If the vector is not empty, sort the vector. Next, create a head ListNode with the first value and a tracking ListNode to add to the head and move after adding a ListNode. Iterate through 
the rest of the vector of sorted values and add on the the linked list using the tracking ListNode. Finally, return the head ListNode.

### Time Complexity
Assume n is the total number of values in all LinkedLists. The first loop would O(n) to loop through all values and insert them into the vector nums. Sorting this vector using C++'s default 
would be O(n log n). The last loop to append all values to the LinkedList which is returned is O(n) complexity. The overall time complexity is O(n log n).

### Space Complexity
The overall space complexity is O(2n) for the vector nums which holds all n values in all LinkedLists and the LinkedList answer which is returned.

<br>

## Trapping Rain Water
<img width="537" alt="Screen Shot 2022-10-03 at 9 41 43 PM" src="https://user-images.githubusercontent.com/32546754/193735908-6c08d766-dbf9-434d-ac5a-2af48a0a3185.png">

### Constraints
<img width="228" alt="Screen Shot 2022-10-03 at 9 42 49 PM" src="https://user-images.githubusercontent.com/32546754/193735980-06e1ba85-8fa6-4ebb-8b3d-f259f7fb4848.png">

### Examples
<img width="530" alt="Screen Shot 2022-10-03 at 9 45 03 PM" src="https://user-images.githubusercontent.com/32546754/193736247-a1d825c2-df35-40e4-90e8-f135990f22b3.png">

### Solution in C++
<img width="463" alt="Screen Shot 2022-10-04 at 9 44 13 PM" src="https://user-images.githubusercontent.com/32546754/193983103-76921fd7-327a-4d02-a8c8-b708554dd989.png">

- Start with left and right trackers and left and right wall variables to keep track of the current greatest values on the left and right side. Then loop through a while loop to calculate the result 
while left tracker is less than right tracker. 
- In the while loop, check if the current height of left is greater than or equal to the current height of right and the height of left is greater than the 
current left_wall, set the new left_wall to the left height. If the current left height is less than the left wall height, add the difference between the left_wall and the current left value to result 
as the water is filling from the left wall towards the right. Then increment left by 1. 
- If current right height is greater than the left wall and the current right height is greater than right_wall, set 
right_wall to the current right height. If the current right height is less than than the right wall, that means water is filling from the right wall towards the left. Add the difference between the 
right_wall and the current right height to the result. Then decrement right by one.
- After the while loop, return the result.

### Time Complexity
Setting the variables is O(1) time. Assuming the length of height is n, the loop is O(n) time complexity, as it runs through n times. So, the overall time complexity is O(n).

### Space Complexity
The space complexity is O(1).

<br>

# Medium Problems
## String to Integer (atoi)
<img width="524" alt="Screen Shot 2022-10-03 at 8 38 08 PM" src="https://user-images.githubusercontent.com/32546754/193729118-c6832a66-f373-4d22-ab05-83396e65b402.png">

### Constraints
<img width="505" alt="Screen Shot 2022-10-03 at 8 38 39 PM" src="https://user-images.githubusercontent.com/32546754/193729176-42dcddea-67fd-47f9-9409-5296a5ac768a.png">

### Examples
<img width="751" alt="Screen Shot 2022-10-03 at 8 39 14 PM" src="https://user-images.githubusercontent.com/32546754/193729248-7914e980-96a7-4593-bf21-0c3ab2d90dc8.png">
<img width="751" alt="Screen Shot 2022-10-03 at 8 39 26 PM" src="https://user-images.githubusercontent.com/32546754/193729267-580eb453-0b44-4c91-892b-332af4cf591e.png">

### Solution in C++
<img width="595" alt="Screen Shot 2022-10-03 at 8 41 06 PM" src="https://user-images.githubusercontent.com/32546754/193729471-06b98de3-7326-4087-a9db-937423847907.png">
To solve this problem, first, initialize a character array of digits. Next, iterate through all characters of the string s. First, skip all white spaces. If the first non white space is '-', change 
the boolean negatigve varibale to true. If the first character is not '+' or '-', set result equal to the first digit. For the rest of the characters, if the character is a digit, first check if the 
result would be greater than the integer maximum value. If so, set the boolean overflow variable to false. Else, multiply the current result by 10 and add the new digit. Finally, return the result 
based on the values of result, and value of the overflow and negative boolean variables.

### Time Complexity
Setting all the variables is O(1) complexity. Assuming the string s has length n, iterating through all characters in the string is O(n) time complexity. The final checks and returns are O(1) time 
complexity, so the overall time complexity is O(n).

### Space Complexity
The overall space complexity is O(1).

<br>

## Walking Robot Simulation II
<img width="525" alt="Screen Shot 2022-10-04 at 9 27 06 PM" src="https://user-images.githubusercontent.com/32546754/193981303-1841c24e-5213-475f-b6f7-775965f0692f.png">

### Constraints
<img width="520" alt="Screen Shot 2022-10-04 at 9 27 44 PM" src="https://user-images.githubusercontent.com/32546754/193981360-df3af632-8d3c-4bc4-bd76-94e4c35ffeae.png">

### Examples
<img width="519" alt="Screen Shot 2022-10-04 at 9 28 11 PM" src="https://user-images.githubusercontent.com/32546754/193981415-15e65478-091d-4861-b97c-27fb6c948824.png">
<img width="498" alt="Screen Shot 2022-10-04 at 9 28 36 PM" src="https://user-images.githubusercontent.com/32546754/193981457-4d76a778-747b-471e-a488-df6681086187.png">

### Solution in C++
<img width="516" alt="Screen Shot 2022-10-04 at 9 42 51 PM" src="https://user-images.githubusercontent.com/32546754/193982893-c1b39286-a619-435d-97a4-f2bbfe7caf4b.png">
<img width="435" alt="Screen Shot 2022-10-04 at 9 43 10 PM" src="https://user-images.githubusercontent.com/32546754/193982916-66e58646-d65d-481a-a834-07948071c428.png">
<img width="256" alt="Screen Shot 2022-10-04 at 9 47 35 PM" src="https://user-images.githubusercontent.com/32546754/193983363-47bb376b-8705-4ecb-a4ca-0a724f010b9c.png">

To solve this problem, I initalized private integer instance variables representing the grid width, grid height, and perimeter size, a string instance variable represeting the direction the robot is facing, and a integer vector represting the coordinate position of the robot. Upon construction of Robot object, set the variables, calculate the perimeter, set the position of the robot to (0, 0), and set the direction to "East". For the getters getPos and getDir, return the integer vector position and the String direction. To move the robot, first check if the number of steps is greater than the perimeter. If so, the moving the robot by the mod of the number of steps is the same as moving the robot by that number of steps. If so, set the direction to South, as that would be the correction direction after iterating through the perimeter. This is your new number of steps. If the number of steps is less than or equal to perimeter, do nothing. After this, loop through a while loop, moving the robot along each direction by the length of the side and turning it towards the correspoding direction. Keep doing this while subtracting the steps by the number of spaces traveled on each side until the number of steps is completed (At this point the current number of steps will be less than or equal to the size of that grid side. 

### Time Complexity
The Constructor and getPos and getDir functions are O(1). The step function is O(1) for the if(num > perimeter) prechecking and the while loop is O(steps / ((length + width) / 2)) for the number of sides traversed and the constant work done on every iteration. So the step function is O(steps / ((length + width) / 2)) overall time complexity.

### Space Complexity
The space complexity for all methods and the constructor is O(1).

## Reverse Integer
<img width="337" alt="Screen Shot 2022-10-04 at 10 44 31 PM" src="https://user-images.githubusercontent.com/32546754/193989684-e2fa330a-3437-42fc-8d14-56461945bd66.png">

### Constraints
<img width="191" alt="Screen Shot 2022-10-04 at 10 43 55 PM" src="https://user-images.githubusercontent.com/32546754/193989598-61d8d886-eaff-4a6d-8e19-c637e5b3dde2.png">

### Examples
<img width="338" alt="Screen Shot 2022-10-04 at 10 44 18 PM" src="https://user-images.githubusercontent.com/32546754/193989650-7c892d4a-91de-475a-bc97-dd237f63ccd8.png">

### Solution in Java
<img width="436" alt="Screen Shot 2022-10-04 at 10 51 56 PM" src="https://user-images.githubusercontent.com/32546754/193990570-98b5551b-5452-4f65-bcc0-e74c175a5ad8.png">

To solve this problem, I first checked if the integer x was equal to zero. If so, return 0. If not, set up a boolean representing whether the number is negative, a reverse_string variable to hold the inverse integer and a string "a" that holds the equivalent string value of x. Next, iterate through the string "a" from back to front, appending each digit to reverse_string. At the last character, check if it is '-'. If so, set the boolean negative to zero. Otherwise, add the character as normal. Finally, return the answer String in Integer form, multiplying by -1 if the boolean negative is true. Also I used try catch blocks to catch numbers which go out the integer range, in which case I return 0.

### Time Complexity
The beginning check is O(1) and all variable initializations are O(1) except for a, which is O(n) - assuming n is the number of digits in x. The for loop is also O(n) to go through all characters in the string representation of the integer x. The final checks and returns are O(1), making the overall time complexity O(n).

### Space Complexity
The space complexity is O(n) since the String a stores the string value of x, so it's length is based off the number of digits in x.

<br>

## Longest Substring Without Repeating Characters
<img width="407" alt="Screen Shot 2022-10-05 at 12 30 23 AM" src="https://user-images.githubusercontent.com/32546754/194005011-134acfc2-56ed-4e33-890f-2311274b8d25.png">

### Constraints
<img width="361" alt="Screen Shot 2022-10-05 at 12 30 53 AM" src="https://user-images.githubusercontent.com/32546754/194005054-7145e6f9-f379-4071-b2a7-c6c399fb9c2d.png">

### Examples
<img width="439" alt="Screen Shot 2022-10-05 at 12 31 12 AM" src="https://user-images.githubusercontent.com/32546754/194005101-a926b587-043f-44de-a68c-a344b2fe5d74.png">

### Solution in Python
<img width="446" alt="Screen Shot 2022-10-05 at 12 36 56 AM" src="https://user-images.githubusercontent.com/32546754/194006063-b7ad6893-47c8-4fbd-a7c5-1c005510f2a7.png">

If the length of s is 0 or 1, return 0 or 1 respectively. Else go through a while loop with a variable i representing the current index. Test a substring starting with the ith index. If the remaining number of character is equal to or less than the current longest substring (result), break. Else, keep adding characters to the current substring (substring) as long as the new character added is not in the current substring. If it is in the current substring, then check if the current substring is greater than the current longest substring (result). If so, replace result with the current substring and increment i so it is after the first duplicate character (No longer substrings are possible before the duplicate chracter). If the for loop reaches the last character, then set the result equal to the current substring (no need to check because of the length check of remaining characters before the for loop). At the end of each loop, increment i. After runing through the while loop , return the length of the greatest substring.

### Time Complexity
Running through the while loop is estimated O(n log n) complexity assuming n is the size of the string s. The outer while loop is O(n) since it may run through every character and the inner for loop is O(log n) since the range is (n / 2) on average. Therefore, the overall time complexity is O(n log n).

### Space Complexity
The overall space complexity is O(1) for the instance variables. 

<br>

# Easy Problems
## Longest Common Prefix
<img width="316" alt="Screen Shot 2022-10-04 at 11 29 09 PM" src="https://user-images.githubusercontent.com/32546754/193995337-5426b872-d90f-4ad9-8fc6-451af31fc406.png">

### Constraints
<img width="316" alt="Screen Shot 2022-10-04 at 11 29 32 PM" src="https://user-images.githubusercontent.com/32546754/193995387-87a56527-27c2-4914-869a-df4cd618115a.png">

### Examples
<img width="331" alt="Screen Shot 2022-10-04 at 11 29 54 PM" src="https://user-images.githubusercontent.com/32546754/193995432-9c4490ed-1f9b-44c4-a4bd-a0d99a3587c3.png">

### Solution in Java
<img width="414" alt="Screen Shot 2022-10-04 at 11 31 13 PM" src="https://user-images.githubusercontent.com/32546754/193995619-9e28fb6d-965c-4b33-b853-295393a3742b.png">
To solve this problem, set a string variable prefix, a boolean variable match, and a length variable. First, loop through the string array to find the length of the smallest word. Next, loop through a for loop the same number of times as the amount of characters in the shortest word. For each character, take the ith character of the first word in the array and compare them to the ith character in every other array. If all characters match, add the character to the prefix. If not, then break the loop. Then return the prefix.

### Time Complexity
Initializing the variables is O(1) complexity. The first for loops is O(n) complexity assuming n is the number of words in array strs. The second array is O(min_length * n) since the outer loop iterates through a maximum min_length times for each character in the mimumum length word in strs. The inner loop loops through n times for each word in strs. So the overall time complexity is O(min_length * n).

### Space Complexity
The overall space complexity is O(1).

<br>

## Palindrome Number
<img width="391" alt="Screen Shot 2022-10-04 at 11 50 54 PM" src="https://user-images.githubusercontent.com/32546754/193998359-16a723ab-91a6-445f-9da8-ecd95fd26e8d.png">

### Constraints
<img width="168" alt="Screen Shot 2022-10-04 at 11 51 26 PM" src="https://user-images.githubusercontent.com/32546754/193998442-59167fc2-32f6-466b-90d0-71ebbc1f8d92.png">

### Examples
<img width="431" alt="Screen Shot 2022-10-04 at 11 51 47 PM" src="https://user-images.githubusercontent.com/32546754/193998498-88105b83-e43d-42bb-90f2-0931f1c698d8.png">

### Solution in Java
<img width="475" alt="Screen Shot 2022-10-04 at 11 52 12 PM" src="https://user-images.githubusercontent.com/32546754/193998569-a3ef6924-e584-4780-ac70-00f6e413d5f4.png">
First, take the string value of the integer and put it in a string variable num. Then get the middle index of the palindrome. Next, loop through a for loop for half the length of the digits starting from 0. For each iteration, check the character at i with the corresponding character which should be in the other half of the string num assuming the string is a palindrome. If the characters do not match, break and return false. Else, return true.

### Time Complexity
Taking the string value of x is O(n) complexity assuming n is the number of digits in x. The for loop is O(n / 2) complexity, as it loops through n / 2 times, or half the number of digits in x. Therefore, the overall time complexity of the program is O(n).

### Space Complexity
The overall space complexity is O(n) since the length of num is based on how many digits are in x.
