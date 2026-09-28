/* CSS cannot select part of a text node, so the trailing "Pro" word in the
 * "Fast DDS Statistics Backend Pro" sidebar caption cannot be turned into a
 * `.pro-badge` tag (see pro_badge.css) with CSS alone. Match the caption by
 * its exact text, wrap that trailing word in a badge span at runtime, and
 * mark the caption with `.caption-wrap` so sidebar.css can scope its
 * wrap-enabling rules to just this one caption instead of every caption.
 */
document.addEventListener('DOMContentLoaded', function () {
    document.querySelectorAll('.wy-menu-vertical p.caption > .caption-text').forEach(function (el) {
        if (el.textContent.trim() === 'Fast DDS Statistics Backend Pro') {
            el.innerHTML = 'Fast DDS Statistics Backend <span class="pro-badge">Pro</span>';
            el.parentElement.classList.add('caption-wrap');
        }
    });
});
