# Explore a compiler decision

Two tickets cost 50 credits. The booking function checks both the available
seats and the credit balance. Which check would you change, and why?

From the checkout root, open the function in a terminal.

```sh
./bin/panack explain tools/explain/samples/booking.panack --function book_pair
```

Statements appear in source order. Use Up and Down to choose a statement.
Enter shows the decisions the compiler made there. Select a decision and press
Enter for its explanation. When a supporting source is available, Enter again
opens it. Left returns to the same selection. Press q to quit.

For the booking example, press **n** to jump directly to the unproved
subtraction. Compare the minimum credit established by the condition with the
cost of the booking. Edit the condition in your copy and repeat the command to
compare the result. A successful compilation also lets you inspect the
instructions generated for a statement. The explorer checks your code without
running it.

Use `--text` for a plain report. Piping the output also selects text mode.
