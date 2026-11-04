# Review the interactive subtraction explanation

Two tickets cost 50 credits. The booking function checks both the available seats
and the credit balance. **Which check would you change, and why?**

From the repository root run:

```sh
./bin/panack explain --interactive tools/explain-review/samples/booking.panack --function book_pair
```

On macOS you can also double-click **Review Panack Explain.command** in this
directory. The first run builds the explorer with the checkout's compiler and a
C compiler. The explorer reads your source on each launch and checks the project
without running the program.

Select a subtraction with the arrows. Enter opens its explanation. If the
compiler retained a branch condition, Enter again shows the source that
established it. Left returns to your selection. Press q to quit.

After you have identified the change, edit your copy of the booking function
and rerun the same command. Both decisions should now be proved.

You can also explore a function from the existing examples:

```sh
./bin/panack explain --interactive examples/fibonacci.panack --function previous
```

This checkout review covers subtraction only. It reads the existing compiler
evidence and uses the ordinary final preparation result for the project status.
Source attribution for generated instructions and the remaining decision
families belong to later slices.
