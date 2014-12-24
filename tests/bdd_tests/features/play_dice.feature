Feature: Create Dice game, buy game chip and play game DICE
  As a regular user
  I have some fund
  I want to buy some game chip and play game DICE
  So that I might win or lose

  Background: Create an asset and a game binding to that asset
    Given I'm Alice
    And I received 5200000 XTS from angel
	And I wait for one block
	And I created a game asset called DICE with precision 100, initial supply 10000, and inital collateral 1000
	And I wait for one block
    And I created a game called DICE with asset DICE
	And I wait for one block

  Scenario: Buy game chip in the game and then play with it
  	Given I'm Bob
  	And I received 500 XTS from angel
  	And I wait for one block
    When I buy for 10 DICE chip
	And I wait for one block
	Then Bob should have 10 DICE
	And I should have 499 XTS minus 1*fee
	When I play game DICE using 10 DICE providing with 2 odds and 1 guess
	And I wait for one block
	Then Bob should have 0 DICE
	When I wait for 9 block
    #Then I should win 20 DICE or lose
	Then I should have 20 or 0 DICE