Feature: Create Dice game, buy game chip and play game DICE
  As a regular user
  I have some fund
  I want to buy some game chip and play game DICE
  So that I might win or lose

  Background: Create an game and a asset accessible to that game
    Given I'm Alice
    And I received 5200000 XTS from angel
	 And I wait for one block
    And I print XTS balance
    And I created a game called dice
	 And I wait for one block
	 And I created a game asset called DICE for dice with precision 0.01, initial supply 10000, and inital collateral 1000
	 And I wait for one block
    And I updated a game called dice
