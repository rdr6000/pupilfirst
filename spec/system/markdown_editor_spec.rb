require 'rails_helper'

feature 'Markdown editor', js: true do
  include UserSpecHelper
  include MarkdownEditorHelper
  include SampleFilesHelper

  # Setup a course with students and target for community.
  let(:school) { create :school, :current }
  let(:course) { create :course, school: school }
  let(:level_1) { create :level, :one, course: course }

  let!(:community) { create :community, school: school, target_linkable: true }
  let(:team) { create :team, level: level_1 }
  let(:student) { create :student, startup: team }

  before do
    create :community_course_connection, course: course, community: community
  end

  let(:first_sentence) { Faker::Lorem.sentence }
  let(:second_sentence) { Faker::Lorem.sentence }

  scenario 'user uploads an image and a PDF' do
    sign_in_user(student.user, referer: new_question_community_path(community))

    expect(page).to have_text('ASK A NEW QUESTION')

    fill_in 'Question', with: 'This is a title.'
    add_markdown(first_sentence)
    attach_file("You can attach files by clicking here and selecting one.", sample_file_path('logo_lipsum_on_light_bg.png'), visible: false)

    expect(page).to have_text('logo_lipsum_on_light_bg.png')

    # Add a bit of sleep here to allow the JS to fully insert the markdown embed code.
    sleep(0.1)

    add_markdown(second_sentence)
    attach_file("You can attach files by clicking here and selecting one.", sample_file_path('pdf-sample.pdf'), visible: false)

    expect(page).to have_text('pdf-sample.pdf')

    click_button('Post Your Question')
    expect(page).to have_text('0 Answers')

    # Let's check if the saved markdown is what we expect...

    last_question = Question.last
    image_attachment = MarkdownAttachment.first
    pdf_attachment = MarkdownAttachment.last

    expect(last_question.description).to include("#{first_sentence}\n![logo_lipsum_on_light_bg.png]")

    expect(last_question.description).to match(
      %r{!\[logo_lipsum_on_light_bg\.png]\(/markdown_attachments/#{image_attachment.id}/[a-zA-Z0-9-_]{22}\)}
    )

    expect(last_question.description).to include("#{second_sentence}\n[pdf-sample.pdf]")

    expect(last_question.description).to match(
      %r{\[pdf-sample\.pdf]\(/markdown_attachments/#{pdf_attachment.id}/[a-zA-Z0-9-_]{22}\)}
    )
  end
end
