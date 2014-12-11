Feature: Buy game chip and play game DICE
  As a regular user
  I have some fund
  I want to buy some game chip and play game DICE
  So that I might win or lose

  Background:
    Given I'm Alice
    And I received 100 XTS from angel
    And Game Dice is active

  Scenario: Buy game chip from market
    When I buy 100 DICE @ 10 DICE/XTS
    And I wait for one block
    Then I should have 90 XTS minus fee
    And I should have 100 DICE
    When I play game Dice with 10 DICE and 50% probability
    Then I have 90 DICE minus fee
    When I wait for one block
    Then I should win 10 DICE or win 0 DICE
